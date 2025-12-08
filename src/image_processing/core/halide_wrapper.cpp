#include "halide_wrapper.h"

#include <Halide.h>

#include <mutex>

// Static member initialization
Halide::Target HalideWrapper::s_bestTarget;
bool HalideWrapper::s_initialized = false;
std::mutex HalideWrapper::s_initMutex;

HalideWrapper::HalideWrapper(Backend backendForce) {
    // 1. Thread-safe initialization
    // We enter this Critical Section if: Global Init: It's the first time running (s_initialized is
    // still false).
    if (!s_initialized) {
        // lock_guard is a mutex wrapper that provides a convenient RAII-style mechanism
        // for owning a mutex for the duration of a scoped block.
        std::lock_guard<std::mutex> Lock(s_initMutex);

        // If not initialized, detect the best target and set initialized to true.
        // s_initialized and s_bestTarget are static member variables that are shared across all
        // instances of the class.
        if (!s_initialized) {
            s_bestTarget = detectBestTarget();  // Will always be GPU if available, otherwise CPU
            s_initialized = true;
        }
    }

    // 2.1 Select Target if forced
    if (backendForce != Backend::AUTO) {
        // WARNING: If you force a backend that is not available on this machine (e.g. CUDA on Mac),
        // Halide will accept the flag here but throw a runtime error later during execution!
        // Use with caution (mostly for debugging or CPU fallback).
        m_target = Halide::get_host_target();  // CPU by default
        switch (backendForce) {
            case Backend::CPU:
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
    }

    // 2.2 Select best target (GPU/CPU found in detectBestTarget) if not forced
    else {
        m_target = s_bestTarget;
    }

// 3. Optimization Flags (Critical for Performance)
// These flags are only enabled in Release mode (NDEBUG is defined)
#ifdef NDEBUG
    // No safety check for math such as zero division, could cause undefined behavior or crash
    m_target.set_feature(Halide::Target::NoAsserts);
    // No checking, if the size & stride of input match the requirement of the compiled code, could
    // cause undefined behavior or crash. For example, if you pass a 1000px width image to a
    // function that expects 1200px width, in this mode it won't check and it will crash.
    m_target.set_feature(Halide::Target::NoBoundsQuery);
#endif

    m_isGPU = m_target.has_gpu_feature();
}

// Detect the best target for Halide based on the system, always GPU if available, otherwise CPU
Halide::Target HalideWrapper::detectBestTarget() {
    Halide::Target target = Halide::get_host_target();  // CPU by default

    // Allow override via Environment Variable
    // Developers can force a specific target strings (e.g. export
    // HL_JIT_TARGET="host-x86-64-debug") from the terminal to test different configurations without
    // recompiling Code. Example: export HL_JIT_TARGET="host-no_asserts-no_bounds_query"
    const char* env_target = getenv("HL_JIT_TARGET");
    if (env_target) {
        return Halide::Target(env_target);
    }

    // 1: Apple Metal
    if (target.os == Halide::Target::OSX) {
        target.set_feature(Halide::Target::Metal);
        std::cout << "[HalideWrapper] Detected Apple Silicon/Mac. Using Metal.\n";
        return target;
    }

    // 2: Nvidia CUDA
    if (Halide::Target::validate_target_string("host-cuda")) {
        target.set_feature(Halide::Target::CUDA);
        std::cout << "[HalideWrapper] Detected Nvidia GPU. Using CUDA.\n";
        return target;
    }

    // 3: OpenCL
    if (Halide::Target::validate_target_string("host-opencl")) {
        target.set_feature(Halide::Target::OpenCL);
        std::cout << "[HalideWrapper] Detected OpenCL GPU. Using OpenCL.\n";
        return target;
    }

    std::cout << "[HalideWrapper] No GPU acceleration found. Using CPU.\n";
    return target;
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

void HalideWrapper::applySchedule(Halide::Func& func, Halide::Var x, Halide::Var y) {
    if (m_isGPU) {
        // --- GPU SCHEDULE (Primary Use Case) ---
        Halide::Var xi, yi;
        // 16x16 tiles are safe for almost all GPUs (Apple, Nvidia, Intel)
        // GuardWithIf prevents crashes on non-multiple image sizes
        func.gpu_tile(x, y, xi, yi, 16, 16, Halide::TailStrategy::GuardWithIf);
    } else {
        // --- CPU SCHEDULE (Debug / Fallback only) ---
        // Since the app uses OpenMP for CPU tasks, this path is not used.
        // Keep it simple: just parallelize rows.
        func.parallel(y);
    }
}