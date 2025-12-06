#include "halide_wrapper.h"

#include <Halide.h>

#include <mutex>

// Static member initialization
Halide::Target HalideWrapper::s_bestTarget;
bool HalideWrapper::s_initialized = false;
std::mutex HalideWrapper::s_initMutex;

HalideWrapper::HalideWrapper(Backend backendForce) {
    // 1. Thread-safe initialization (Runs only one)
    if (!s_initialized || backendForce != Backend::AUTO) {
        std::lock_guard<std::mutex> Lock(s_initMutex);
        if (!s_initialized) {
            s_bestTarget = detectBestTarget();
            s_initialized = true;
        }
    }

    // 2. Select Target
    if (backendForce != Backend::AUTO) {
        m_target = Halide::get_host_target();
        switch (backendForce) {
            case Backend::CPU: /* Only CPU */
                break;
            case Backend::CUDA:
                m_target.set_feature(Halide::Target::CUDA);
                break;
            case Backend::METAL:
                m_target.set_feature(Halide::Target::Metal);
                break;
            case Backend::OPENCL:
                m_target.set_feature(Halide::Target::OpenCL);
                break;
            default:
                break;
        }
    } else {
        m_target = s_bestTarget;
    }

// 3. Optimization Flags (Critical for Performance)
#ifdef NDEBUG
    // No safety check for math such as zero division
    m_target.set_feature(Halide::Target::NoAsserts);
    // No checking, if the size & stride of input match the requirement of the compiled code
    m_target.set_feature(Halide::Target::NoBoundsQuery);
#endif

    m_isGPU = m_target.has_gpu_feature();
}

Halide::Target HalideWrapper::detectBestTarget() {
    Halide::Target t = Halide::get_host_target();

    // Allow override via Environment Variable (e.g. for testing on different machines)
    const char* env_target = getenv("HL_JIT_TARGET");
    if (env_target) {
        return Halide::Target(env_target);
    }

    // Priority 1: Apple Metal
    if (t.os == Halide::Target::OSX) {
        t.set_feature(Halide::Target::Metal);
        std::cout << "[HalideWrapper] Detected Apple Silicon/Mac. Using Metal.\n";
        return t;
    }

    // Priority 2: Nvidia CUDA
    if (Halide::Target::validate_target_string("host-cuda")) {
        t.set_feature(Halide::Target::CUDA);
        std::cout << "[HalideWrapper] Detected Nvidia GPU. Using CUDA.\n";
        return t;
    }

    // Priority 3: OpenCL
    if (Halide::Target::validate_target_string("host-opencl")) {
        t.set_feature(Halide::Target::OpenCL);
        std::cout << "[HalideWrapper] Detected OpenCL GPU. Using OpenCL.\n";
        return t;
    }

    std::cout << "[HalideWrapper] No GPU acceleration found. Using CPU.\n";
    return t;
}

Halide::Target HalideWrapper::getTarget() const {
    return m_target;
}

bool HalideWrapper::isGPU() const {
    return m_isGPU;
}

std::string HalideWrapper::getTargetName() const {
    return m_target.to_string();
}

void HalideWrapper::applySchedule(Halide::Func& f, Halide::Var x, Halide::Var y) {
    if (m_isGPU) {
        // --- GPU SCHEDULE (Primary Use Case) ---
        Halide::Var xi, yi;
        // 16x16 tiles are safe for almost all GPUs (Apple, Nvidia, Intel)
        // GuardWithIf prevents crashes on non-multiple image sizes
        f.gpu_tile(x, y, xi, yi, 16, 16, Halide::TailStrategy::GuardWithIf);
    } else {
        // --- CPU SCHEDULE (Debug / Fallback only) ---
        // Since the app uses OpenMP for CPU tasks, this path is rarely used.
        // Keep it simple: just parallelize rows.
        f.parallel(y);
    }
}