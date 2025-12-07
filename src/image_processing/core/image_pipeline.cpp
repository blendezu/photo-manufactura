#include "image_pipeline.h"

#include <Halide.h>

#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <ostream>
#include <sstream>

#include "halide_wrapper.h"
#include "operation_base.h"

ImagePipeline::ImagePipeline() : m_cacheValid(false), m_useFusedPipeline(false) {}

void ImagePipeline::setImg(const cv::Mat& img) {
    if (img.empty()) {
        std::cerr << "Warning: Setting empty image to pipeline\n";
        return;
    }

    m_originalImg = img.clone();
    invalidateCache();
    clearUndoHistory();
    std::cout << "Image set to pipeline: " << img.cols << "x" << img.rows
              << " channels: " << img.channels() << std::endl;
}

void ImagePipeline::addOperation(std::shared_ptr<ImageOperation> operation) {
    if (!operation) {
        std::cerr << "Error: cannot add null operation to pipeline\n";
        return;
    }

    m_operations.push_back(operation);
    m_undoneOperations.clear();  // delete Redo history when new operation
    invalidateCache();

    std::cout << "Operation added: " << operation->getName() << " (Total: " << m_operations.size()
              << ")" << std::endl;
}

void ImagePipeline::insertOperation(int index, std::shared_ptr<ImageOperation> operation) {
    if (!operation) {
        std::cerr << "Error: Cannot insert null operation\n";
        return;
    }

    if (index < 0 || index > static_cast<int>(m_operations.size())) {
        std::cerr << "Error: Invalid index for operation insertion: " << index << std::endl;
        return;
    }

    m_operations.insert(m_operations.begin() + index, operation);
    m_undoneOperations.clear();
    invalidateCache();

    std::cout << "Operation inserted at " << index << ": " << operation->getName() << std::endl;
}

void ImagePipeline::removeOperation(int index) {
    if (index < 0 || index >= static_cast<int>(m_operations.size())) {
        std::cerr << "Error: Invalid index for operation removal: " << index << std::endl;
        return;
    }

    std::string opName = m_operations[index]->getName();
    m_operations.erase(m_operations.begin() + index);
    invalidateCache();

    std::cout << "Operation removed at " << index << ": " << opName << std::endl;
}

void ImagePipeline::clearOperations() {
    if (!m_operations.empty()) {
        std::cout << "Clearing all operations (" << m_operations.size() << " total)" << std::endl;
        m_operations.clear();
        invalidateCache();
    }
}

void ImagePipeline::clearUndoHistory() {
    if (!m_undoneOperations.empty()) {
        m_undoneOperations.clear();
        std::cout << "Undo history cleared" << std::endl;
    }
}

std::shared_ptr<ImageOperation> ImagePipeline::getOperation(int index) {
    if (index < 0 || index >= static_cast<int>(m_operations.size())) {
        std::cerr << "Error: Invalid operation index: " << index << std::endl;
        return nullptr;
    }

    return m_operations[index];
}

void ImagePipeline::setLiveOperation(std::shared_ptr<ImageOperation> operation) {
    m_liveOperation = operation;
}

void ImagePipeline::clearLiveOperations() {
    m_liveOperation = nullptr;
}

void ImagePipeline::setFusionMode(bool enabled) {
    if (m_useFusedPipeline != enabled) {
        m_useFusedPipeline = enabled;
        invalidateCache();
        std::cout << "[Pipeline] Mode switched to: " << (enabled ? "GPU Fused" : "CPU Sequential")
                  << std::endl;
    }
}

cv::Mat ImagePipeline::process() {
    if (m_originalImg.empty()) {
        std::cerr << "Error: Cannot process pipeline - no image loaded" << std::endl;
        return cv::Mat();
    }

    // 1. Check Cache
    if (m_cacheValid && !m_cachedResult.empty() && !m_liveOperation) {
        return m_cachedResult.clone();
    }

    cv::Mat result;

    // 2. Reuse Cache or  Calculate fresh
    if (m_cacheValid && !m_cachedResult.empty()) {
        result = m_cachedResult.clone();
    } else {
        if (m_useFusedPipeline) {
            result = processFused();
        }

        else {
            result = processSequential();
        }

        updateCache(result);
    }

    // 3. Live Operation Overlay
    if (m_liveOperation) {
        cv::Mat previous = result.clone();

        // Apply Live Operation
        cv::Mat liveResult = m_liveOperation->apply(result);

        if (liveResult.empty()) {
            std::cerr << "[Pipeline] Error: Live operation returned empty image" << std::endl;
            result = previous;
        } else {
            result = liveResult;
        }
    }
    return result;
}

// --- ENGINE 1: SEQUENTIAL ---
cv::Mat ImagePipeline::processSequential() {
    cv::Mat result = m_originalImg.clone();

    try {
        for (size_t i = 0; i < m_operations.size(); i++) {
            auto& operation = m_operations[i];
            if (!operation) {
                std::cerr << "Warning: Null operation at index " << i << ", skipping" << std::endl;
                continue;
            }

            cv::Mat previous = result.clone();
            result = operation->apply(result);

            if (result.empty()) {
                std::cerr << "Error: Operation " << operation->getName() << " at index " << i
                          << " returned empty image\n";
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during sequential processing: " << e.what() << std::endl;
        return m_originalImg.clone();
    }
    return result;
}

// --- ENGINE 2: FUSED ---
cv::Mat ImagePipeline::processFused() {
    cv::Mat currentImg = m_originalImg.clone();

    // Queue for operations that can be used on GPU
    std::vector<std::shared_ptr<HalideOperation>> halideChain;

    // Helper lambda to execute pending GPU ops
    auto flushChain = [&](cv::Mat& img) {
        if (halideChain.empty()) {
            return;
        }

        try {
            img = runFusedHalideChain(img, halideChain);
        } catch (const std::exception& e) {
            std::cerr << "[Pipeline] Chain execution failed: " << e.what() << std::endl;
        }

        // clear the pipeline
        halideChain.clear();
    };

    for (const auto& op : m_operations) {
        if (!op)
            continue;

        // Check compatbility
        if (op->supportsHalide()) {
            auto halideOp = std::dynamic_pointer_cast<HalideOperation>(op);
            if (halideOp) {
                //
                if (halideOp->requiresFreshStats()) {
                    flushChain(currentImg);
                }
                // IMPORTANT: Prepare stats/parameters on CPU befor queuing
                halideOp->prepareParameters(currentImg);
                halideChain.push_back(halideOp);
                continue;
            }
        }

        // --- BARRIER: CPU Filter encountered

        // 1. Finishing pending GPU work
        flushChain(currentImg);

        // 2. Run the CPU operation
        try {
            cv::Mat next = op->apply(currentImg);
            if (!next.empty()) {
                currentImg = next;
            } else {
                std::cerr << "[Pipeline] Warning: CPU Op returned empty" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[Pipeline] CPU Fallback Error: " << e.what() << std::endl;
        }
    }

    // Finish remaining GPU work
    flushChain(currentImg);

    return currentImg;
}

cv::Mat ImagePipeline::runFusedHalideChain(const cv::Mat& src,
                                           std::vector<std::shared_ptr<HalideOperation>>& ops) {
    if (ops.empty()) {
        return src;
    }

    HalideWrapper hw;
    int depth = src.depth();
    int channels = src.channels();
    bool is8Bit = (depth == CV_8U);

    try {
        // --- 1. CHECK CACHE VALIDITY ---
        bool cacheHit = true;

        // A. Check if input format changed
        if (m_pipelineCache.inputDepth != depth || m_pipelineCache.inputChannels != channels) {
            cacheHit = false;
        }
        // B. Check if operation sequence changed
        else if (m_pipelineCache.ops.size() != ops.size()) {
            cacheHit = false;
        } else {
            for (size_t i = 0; i < ops.size(); i++) {
                if (m_pipelineCache.ops[i] != ops[i]) {
                    cacheHit = false;
                    break;
                }
            }
        }

        // --- 2. REBUILD PIPELINE IF NEEDED (Cache Miss) ---
        if (!cacheHit) {
            // std::cout << "[Pipeline] JIT Cache Miss - Recompiling..." << std::endl;

            m_pipelineCache.ops = ops;
            m_pipelineCache.inputDepth = depth;
            m_pipelineCache.inputChannels = channels;

            Halide::Var x("x"), y("y"), c("c");

            // Create ImageParam
            Halide::Type inputType = is8Bit ? Halide::UInt(8) : Halide::UInt(16);
            m_pipelineCache.inputParam = Halide::ImageParam(inputType, 3, "input_img");

            // CRITICAL FIX: Set Stride Constraints for Interleaved Layout
            // Halide defaults to Planar (stride(0)=1) unless specified.
            // OpenCV Interleaved: stride(0) = channels, stride(2) = 1
            m_pipelineCache.inputParam.dim(0).set_stride(channels);
            m_pipelineCache.inputParam.dim(2).set_stride(1);
            m_pipelineCache.inputParam.dim(2).set_bounds(0, channels);

            // Define Input with Boundary Conditions
            Halide::Func currentFunc("chain_start");
            currentFunc(x, y, c) =
                Halide::BoundaryConditions::repeat_edge(m_pipelineCache.inputParam)(x, y, c);

            // Build Chain
            for (auto& op : ops) {
                currentFunc = op->applyHalide(currentFunc, x, y, c);
            }

            // Output Handling
            Halide::Func finalFunc("chain_end");
            float maxVal = is8Bit ? 255.0f : 65535.0f;

            if (is8Bit) {
                finalFunc(x, y, c) =
                    Halide::cast<uint8_t>(Halide::clamp(currentFunc(x, y, c), 0.0f, maxVal));
            } else {
                finalFunc(x, y, c) =
                    Halide::cast<uint16_t>(Halide::clamp(currentFunc(x, y, c), 0.0f, maxVal));
            }

            // Memory Layout (Interleaved)
            finalFunc.output_buffer().dim(0).set_stride(channels);
            finalFunc.output_buffer().dim(2).set_stride(1);

            // Scheduling
            finalFunc.bound(c, 0, channels);
            finalFunc.reorder(c, x, y).unroll(c);

            hw.applySchedule(finalFunc, x, y);

            // Store compilation result
            m_pipelineCache.pipeline = Halide::Pipeline(finalFunc);
        }
        // else { std::cout << "[Pipeline] JIT Cache Hit!" << std::endl; }

        // --- 3. EXECUTION ---
        cv::Mat dst = src.clone();

        // Wrap Inputs & Outputs
        if (is8Bit) {
            auto inBuf = hw.wrap<uint8_t>(src);
            auto outBuf = hw.wrap<uint8_t>(dst);

            // Bind Input
            m_pipelineCache.inputParam.set(inBuf);

            // Run
            if (is8Bit)
                inBuf.set_host_dirty();

            m_pipelineCache.pipeline.realize(outBuf, hw.getTarget());

            if (hw.isGPU()) {
                outBuf.copy_to_host();
                outBuf.device_sync();
            }
        } else {
            auto inBuf = hw.wrap<uint16_t>(src);
            auto outBuf = hw.wrap<uint16_t>(dst);

            m_pipelineCache.inputParam.set(inBuf);

            if (!is8Bit)
                inBuf.set_host_dirty();

            m_pipelineCache.pipeline.realize(outBuf, hw.getTarget());

            if (hw.isGPU()) {
                outBuf.copy_to_host();
                outBuf.device_sync();
            }
        }

        return dst;

    } catch (const Halide::Error& e) {
        std::cerr << "[Pipeline] Halide Implementation Error: " << e.what() << std::endl;
        // Invalidate cache on error to force rebuild next time
        m_pipelineCache.ops.clear();
        return src;
    } catch (const std::exception& e) {
        std::cerr << "[Pipeline] Fused Runtime Error: " << e.what() << std::endl;
        m_pipelineCache.ops.clear();
        return src;
    }
}

cv::Mat ImagePipeline::processUpTo(int operationIndex) {
    if (m_originalImg.empty()) {
        std::cerr << "Error: Cannot process pipeline - no image loaded\n";
        return cv::Mat();
    }

    if (operationIndex < 0 || operationIndex > static_cast<int>(m_operations.size())) {
        std::cerr << "Error: Invalid operation index for partial processing: " << operationIndex
                  << std::endl;
        return process();
    }

    cv::Mat result = m_originalImg.clone();

    try {
        for (int i = 0; i < operationIndex; i++) {
            auto& operation = m_operations[i];
            if (!operation) {
                std::cerr << "Warning: Null operation at index " << i << ", skipping" << std::endl;
                continue;
            }

            result = operation->apply(result);

            if (result.empty()) {
                std::cerr << "Error: Operation " << operation->getName() << " at index " << i
                          << "returned empty image\n";
                return m_originalImg.clone();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during partial pipeline processing: " << e.what() << std::endl;
        return m_originalImg.clone();
    }

    return result;
}

void ImagePipeline::undo() {
    if (m_operations.empty()) {
        std::cout << "Nothing to undo\n";
        return;
    }

    auto lastOp = m_operations.back();
    m_undoneOperations.push_back(lastOp);
    m_operations.pop_back();
    invalidateCache();

    std::cout << "Undo: " << lastOp->getName() << std::endl;
}

void ImagePipeline::redo() {
    if (m_undoneOperations.empty()) {
        std::cout << "Nothing to redo" << std::endl;
        return;
    }

    auto lastUndone = m_undoneOperations.back();
    m_operations.push_back(lastUndone);
    m_undoneOperations.pop_back();
    invalidateCache();

    std::cout << "Redo: " << lastUndone->getName() << std::endl;
}

void ImagePipeline::invalidateCache() {
    m_cacheValid = false;
    m_cachedResult.release();
}

void ImagePipeline::updateCache(const cv::Mat& result) {
    if (!result.empty()) {
        m_cachedResult = result.clone();
        m_cacheValid = true;
    }
}

std::string ImagePipeline::serializePipeline() const {
    std::stringstream ss;
    ss << "ImagePipeline v1.0|";
    ss << "Operations:" << m_operations.size() << "|";

    for (size_t i = 0; i < m_operations.size(); i++) {
        if (m_operations[i]) {
            ss << m_operations[i]->getName() << ":" << m_operations[i]->getSettings();
            if (i < m_operations.size() - 1) {
                ss << ";";
            }
        }
    }

    return ss.str();
}

void ImagePipeline::deserializePipeline(const std::string& data) {
    std::cout << "Deserializing pipeline: " << data.substr(0, 50) << "..." << std::endl;
}