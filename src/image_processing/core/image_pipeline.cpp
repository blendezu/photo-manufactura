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

/**
 * @brief Constructs a new ImagePipeline object.
 * Cache is invalid and fused pipeline is disabled by default.
 */
ImagePipeline::ImagePipeline() : m_cacheValid(false), m_useFusedPipeline(false) {}

void ImagePipeline::setImg(const cv::Mat& img) {
    // 1. Check if image is empty
    if (img.empty()) {
        std::cerr << "[ImagePipeline] Error: Setting empty image to pipeline\n";
        return;
    }

    // 2. Set image to pipeline. Clone to avoid modifying original image
    m_originalImg = img.clone();

    // 3. Invalidate cache and clear undo history for new image
    invalidateCache();
    clearUndoHistory();

    // 4. Print image info
    std::cout << "[ImagePipeline] Image set to pipeline: " << img.cols << "x" << img.rows
              << " channels: " << img.channels() << std::endl;
}

// --- B. Operation Stack Management ---

void ImagePipeline::addOperation(std::shared_ptr<ImageOperation> operation) {
    if (!operation) {
        std::cerr << "[ImagePipeline] Error: cannot add null operation to pipeline\n";
        return;
    }

    // 1. Get index for new operation
    int index = static_cast<int>(m_operations.size());

    // 2. Add operation to pipeline stack
    m_operations.push_back(operation);

    // 3. Record Action (INSERT at end)
    m_undoStack.push_back({PipelineAction::INSERT, index, operation, nullptr});

    // 4. Clear redo history to avoid conflicts
    m_redoStack.clear();

    // 5. Invalidate cache to force reprocessing
    invalidateCache();

    // 6. Print info
    std::cout << "[ImagePipeline] Operation added: " << operation->getName()
              << " (Total: " << m_operations.size() << ")\n";
}

void ImagePipeline::insertOperation(int index, std::shared_ptr<ImageOperation> operation) {
    if (!operation) {
        std::cerr << "[ImagePipeline] Error: Cannot insert null operation\n";
        return;
    }
    if (index < 0 || index > static_cast<int>(m_operations.size())) {
        std::cerr << "[ImagePipeline] Error: Invalid index for operation insertion: " << index
                  << "\n";
        return;
    }

    // 1. Insert operation at index
    m_operations.insert(m_operations.begin() + index, operation);

    // 2. Record Action (INSERT at index)
    m_undoStack.push_back({PipelineAction::INSERT, index, operation, nullptr});

    // 3. Clear redo history
    m_redoStack.clear();

    invalidateCache();
    std::cout << "[ImagePipeline] Operation inserted at " << index << ": " << operation->getName()
              << "\n";
}

void ImagePipeline::removeOperation(int index) {
    if (index < 0 || index >= static_cast<int>(m_operations.size())) {
        std::cerr << "[ImagePipeline] Error: Invalid index for operation removal: " << index
                  << "\n";
        return;
    }

    // 1. Save state for Undo
    auto opToRemove = m_operations[index];
    std::string opName = opToRemove->getName();

    // 2. Remove operation
    m_operations.erase(m_operations.begin() + index);

    // 3. Record Action (REMOVE at index)
    m_undoStack.push_back({PipelineAction::REMOVE, index, opToRemove, nullptr});

    // 4. Clear redo history
    m_redoStack.clear();

    invalidateCache();
    std::cout << "[ImagePipeline] Operation removed at " << index << ": " << opName << "\n";
}

void ImagePipeline::modifyOperation(int index, std::shared_ptr<ImageOperation> newOp) {
    if (index < 0 || index >= static_cast<int>(m_operations.size())) {
        std::cerr << "[ImagePipeline] Error: Invalid index for modify: " << index << "\n";
        return;
    }
    if (!newOp)
        return;

    // 1. Save old state
    auto oldOp = m_operations[index];

    // 2. Replace operation
    m_operations[index] = newOp;

    // 3. Record Action (MODIFY: Needs both old and new)
    m_undoStack.push_back({PipelineAction::MODIFY, index, newOp, oldOp});

    // 4. Clear redo history
    m_redoStack.clear();

    // 5. Invalidate cache to force reprocessing
    invalidateCache();

    // 6. Print info
    std::cout << "[ImagePipeline] Operation " << newOp->getName() << " modified at " << index
              << "\n";
}

void ImagePipeline::clearOperations() {
    if (!m_operations.empty()) {
        std::cout << "[ImagePipeline] Clearing all operations (" << m_operations.size() << " total)"
                  << "\n";

        // 1. Clear operations
        m_operations.clear();

        // 2. Clear history
        m_undoStack.clear();
        m_redoStack.clear();

        // 3. Invalidate cache to force reprocessing
        invalidateCache();
    }
}

void ImagePipeline::clearUndoHistory() {
    // 1. Clear history
    m_undoStack.clear();
    m_redoStack.clear();

    // 2. Print info
    std::cout << "[ImagePipeline] History cleared\n";
}

// getOperation helpers
std::shared_ptr<ImageOperation> ImagePipeline::getOperation(int index) {
    if (index < 0 || index >= static_cast<int>(m_operations.size())) {
        std::cerr << "Error: Invalid operation index: " << index << "\n";
        return nullptr;
    }
    return m_operations[index];
}

// Live Operations
void ImagePipeline::setLiveOperation(std::shared_ptr<ImageOperation> operation) {
    m_liveOperation = operation;
}

void ImagePipeline::clearLiveOperations() {
    m_liveOperation = nullptr;
}

void ImagePipeline::setFusionMode(bool enabled) {
    if (m_useFusedPipeline != enabled) {
        // 1. Set fusion mode
        m_useFusedPipeline = enabled;

        // 2. Invalidate cache to force reprocessing
        invalidateCache();

        // 3. Print info
        std::cout << "[Pipeline] Mode switched to: " << (enabled ? "GPU Fused" : "CPU Sequential")
                  << "\n";
    }
}

cv::Mat ImagePipeline::process() {
    // Check for empty image
    if (m_originalImg.empty()) {
        std::cerr << "Error: Cannot process pipeline - no image loaded\n";
        return cv::Mat();
    }

    // --- A. CACHE AND LIVE OPERATION CHECK ---
    // Check cache, if valid and not in live mode, return cached result
    // This is to avoid reprocessing the image if it has not changed
    // This is useful for live operations and resizing GUI
    if (m_cacheValid && !m_cachedResult.empty() && !m_liveOperation) {
        return m_cachedResult.clone();
    }

    // --- B. PROCESS IMAGE ---
    // Output image
    cv::Mat result;

    // B1. Use cached result as input for a new operation if valid and not in live mode, so the
    // whole pipeline doesn't need to be reprocessed
    if (m_cacheValid && !m_cachedResult.empty()) {
        result = m_cachedResult.clone();
    }

    // B2. If cache is invalid, process image
    else {
        // B2.1. Process image if fusion mode is enabled
        if (m_useFusedPipeline) {
            result = processFused();
        }
        // B2.2. Process image if fusion mode is disabled
        else {
            result = processSequential();
        }

        // B2.3. Update cache
        updateCache(result);
    }

    // B3. Apply live operation
    if (m_liveOperation) {
        cv::Mat previous = result.clone();
        cv::Mat liveResult = m_liveOperation->apply(result);
        if (liveResult.empty()) {
            std::cerr << "[Pipeline] Error: Live operation returned empty image\n";
            result = previous;
        } else {
            result = liveResult;
        }
    }

    return result;
}

// --- ENGINE 1: SEQUENTIAL ---
cv::Mat ImagePipeline::processSequential() {
    // 1. Clone original image to avoid modifying it
    cv::Mat result = m_originalImg.clone();

    try {
        for (size_t i = 0; i < m_operations.size(); i++) {
            // 2. Get operation
            auto& operation = m_operations[i];

            // 3. Check if operation is null
            if (!operation) {
                std::cerr << "Warning: Null operation at index " << i << ", skipping\n";
                continue;
            }

            // 5. Apply operation
            try {
                // IMPORTANT: do NOT clone 'result' here for performance.
                // Since apply() returns a NEW image and takes srcImg as const ref,
                // 'result' remains untouched until we explicitly overwrite it.
                cv::Mat nextResult = operation->apply(result);

                if (nextResult.empty()) {
                    std::cerr << "Error: Operation " << operation->getName() << " at index " << i
                              << " returned empty image. Skipping step.\n";
                    // Do nothing. 'result' still holds the image from step i-1.
                } else {
                    result = nextResult;
                }
            } catch (const std::exception& e) {
                std::cerr << "Warning: Operation " << operation->getName()
                          << " threw exception: " << e.what() << ". Skipping this operation.\n";
                // 'result' remains untouched (state from step i-1)
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Critical Error during sequential processing: " << e.what() << "\n";
        return result;  // Return the result we have so far
    }
    return result;
}

// --- ENGINE 2: FUSED ---
// There are two factors that stop the fused pipeline from being used:
// 1. The operation cannot be executed on GPU
// 2. The operation is not a Halide operation
// In that case the pipeline need to be flushed and move data to CPU
cv::Mat ImagePipeline::processFused() {
    // 1. Clone original image to avoid modifying it
    cv::Mat currentImg = m_originalImg.clone();

    // 2. Queue for operations that can be used on GPU
    std::vector<std::shared_ptr<HalideOperation>> halideChain;

    // 3. Helper lambda to execute pending GPU ops
    // This lambda function processes a batch of accumulated Halide operations.
    // It checks if the 'halideChain' vector contains any operations. If so, it
    // executes them as a fused pipeline using 'runFusedHalideChain' on the input
    // image 'img'. This allows for efficient GPU execution by minimizing memory
    // transfers and kernel launches. After execution or if an error occurs,
    // the chain is cleared to prepare for the next sequence of operations.
    auto flushChain = [&](cv::Mat& img) {
        // 3.1. Check if there are any operations to execute
        if (halideChain.empty()) {
            return;
        }

        // 3.2. Execute pending GPU ops
        try {
            img = runFusedHalideChain(img, halideChain);
        } catch (const std::exception& e) {
            std::cerr << "[Pipeline] Chain execution failed: " << e.what() << "\n";
        }

        // 3.3. Clear chain to prepare for next sequence of operations
        halideChain.clear();
    };

    // 4. Process operations
    // If the operation can be executed on GPU, it will be added to the chain and executed all of
    // them later.
    // If the operation cannot be executed on GPU, the chain will be executed and the
    // operation will be executed on CPU.
    for (const auto& op : m_operations) {
        // 4.1. Skip null operations
        if (!op)
            continue;

        // 4.2. If operation can be executed on GPU:
        if (op->supportsHalide()) {
            // 4.2.1. Cast to HalideOperation
            // Operation is the base class for all operations. It does not have Halide-specific
            // methods like prepareParameters() and requiresFreshStats(), so we need to cast it to
            // HalideOperation to access Halide-specific methods.
            auto halideOp = std::dynamic_pointer_cast<HalideOperation>(op);

            // 4.2.2. If cast was successful (i.e. operation is a HalideOperation):
            if (halideOp) {
                // Before being executed on GPU some operations require fresh Stats, which are
                // computed on the CPU.
                // 4.2.3. If operation requires fresh stats, execute pending GPU ops
                if (halideOp->requiresFreshStats()) {
                    // The available operations in the chain need will be flushed (GPU), because it
                    // needs the result of the available operations.
                    flushChain(currentImg);
                }

                // 4.2.4. Prepare parameters for Halide operation (on CPU)
                halideOp->prepareParameters(currentImg);

                // 4.2.5. Add operation to chain to create another chain
                halideChain.push_back(halideOp);
                continue;
            }
        }

        // 4.3. If operation cannot be executed on GPU, execute pending GPU ops
        flushChain(currentImg);

        // 4.4. Execute operation on CPU
        try {
            cv::Mat next = op->apply(currentImg);
            if (!next.empty()) {
                currentImg = next;
            } else {
                std::cerr << "[Pipeline] Warning: CPU Op returned empty\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[Pipeline] CPU Fallback Error: " << e.what() << "\n";
        }
    }

    // 4.5. Execute GPU ops
    flushChain(currentImg);

    // 4.6. Return result
    return currentImg;
}

cv::Mat ImagePipeline::runFusedHalideChain(const cv::Mat& srcImg,
                                           std::vector<std::shared_ptr<HalideOperation>>& ops) {
    // 1. Check if there are any operations to execute
    if (ops.empty()) {
        return srcImg;
    }

    // 2. Initialize HalideWrapper
    HalideWrapper halideWrapper;

    // 3. Prepare image parameters
    int depth = srcImg.depth();
    int channels = srcImg.channels();
    bool is8Bit = (depth == CV_8U);
    std::cerr << "[Pipeline debug] runFusedHalideChain Start. Depth: " << depth << " Is8Bit: " << is8Bit << std::endl;

    // Calculate Output Dimensions (Needed for Bounds)
    int currentWidth = srcImg.cols;
    int currentHeight = srcImg.rows;
    for (const auto& op : ops) {
        int nextW, nextH;
        op->getOutputDimensions(currentWidth, currentHeight, nextW, nextH);
        currentWidth = nextW;
        currentHeight = nextH;
    }

    try {
        // --- 4. CHECK CACHE VALIDITY ---
        bool cacheHit = true;

        // A. Check if input format changed
        // CRITICAL: Compiled Halide code has HARDCODED memory strides (e.g. 1 byte step for 8-bit,
        // 2 bytes for 16-bit). If we apply 8-bit compiled code to a 16-bit image (or vice versa),
        // it will misinterpret pixel boundaries and cause severe memory corruption (Segfaults) or
        // garbage output. This check ensures we recompile if the user switches image types (e.g.
        // JPG -> RAW).
        if (m_pipelineCache.inputDepth != depth || m_pipelineCache.inputChannels != channels) {
            cacheHit = false;
        }

        // B. Check if operation sequence changed
        else if (m_pipelineCache.ops.size() != ops.size()) {
            cacheHit = false;
        }

        // C. Check if operations changed
        else {
            for (size_t i = 0; i < ops.size(); i++) {
                if (m_pipelineCache.ops[i] != ops[i]) {
                    cacheHit = false;
                    break;
                }
            }
        }

        // CRITICAL: If any of the above checks fail, we need to rebuild the pipeline.
        // --- 5. REBUILD PIPELINE IF NEEDED (Cache Miss) ---
        if (!cacheHit) {
            // 5.1. Rebuild pipeline cache
            m_pipelineCache.ops = ops;
            m_pipelineCache.inputDepth = depth;
            m_pipelineCache.inputChannels = channels;

            // 5.2. Rebuild Halide pipeline
            Halide::Var x("x"), y("y"), c("c");

            // 5.3. Create ImageParam
            Halide::Type inputType = is8Bit ? Halide::UInt(8) : Halide::UInt(16);
            m_pipelineCache.inputParam = Halide::ImageParam(inputType, 3, "input_img");

            // 5.4. CRITICAL: Set Stride Constraints for Interleaved Layout
            // Halide defaults to Planar (stride(0)=1) unless specified.
            // OpenCV Interleaved: stride(0) = channels, stride(2) = 1
            // m_pipelineCache.inputParam.dim(0).set_stride(channels);                                    // 3 strides to go to next pixel
            // m_pipelineCache.inputParam.dim(2).set_stride(1);  // 1 stride to go to color channel
            // m_pipelineCache.inputParam.dim(2).set_bounds(0, channels);

            // 5.5. Define Input with Boundary Conditions
            Halide::Func currentFunc("chain_start");
            // "repeat_edge" handles out-of-bounds access (e.g. x=-1 becomes x=0).
            // This prevents crashes when filters look at neighbors outside the image.
            currentFunc(x, y, c) =
                Halide::BoundaryConditions::repeat_edge(m_pipelineCache.inputParam)(x, y, c);

            // 5.6. Build Chain
            for (auto& op : ops) {
                std::cout << "[" << op->getName() << "] Using Halide" << std::endl;
                currentFunc = op->applyHalide(currentFunc, x, y, c);
            }

            // 5.7. Output Handling
            Halide::Func finalFunc("chain_end");
            float maxVal = is8Bit ? 255.0f : 65535.0f;

            if (is8Bit) {
                finalFunc(x, y, c) =
                    Halide::cast<uint8_t>(Halide::clamp(currentFunc(x, y, c), 0.0f, maxVal));
            } else {
                finalFunc(x, y, c) =
                    Halide::cast<uint16_t>(Halide::clamp(currentFunc(x, y, c), 0.0f, maxVal));
            }

            // 5.8. Memory Layout (Interleaved)
            finalFunc.output_buffer().dim(0).set_stride(channels);
            finalFunc.output_buffer().dim(2).set_stride(1);

            // 5.9. Scheduling
            finalFunc.bound(c, 0, channels);
            finalFunc.reorder(c, x, y).unroll(c);

            halideWrapper.applySchedule(finalFunc, x, y);

            // 5.10. Store compilation result
            m_pipelineCache.pipeline = Halide::Pipeline(finalFunc);
        }

        // --- 6. EXECUTION ---

        // 6.1. Calculate Output Dimensions & Create Destination Image
        // This is neccessary because of operations with crop like Crop or Rotation
        // int currentWidth = srcImg.cols;
        // int currentHeight = srcImg.rows;
        //
        // for (const auto& op : ops) {
        //     int nextW, nextH;
        //     op->getOutputDimensions(currentWidth, currentHeight, nextW, nextH);
        //     currentWidth = nextW;
        //     currentHeight = nextH;
        // }

        // Create destination image with correct size
        cv::Mat dstImg;
        dstImg.create(currentHeight, currentWidth, srcImg.type());

        // 6.2. 8-bit Image
        if (is8Bit) {
            std::cerr << "[Pipeline debug] 6.2. 8-bit Image branch entered" << std::endl;
            // 6.2.1 Wrap Inputs & Outputs using Halide Wrapper.
            // inBuf and outBuf are Halide::Buffer objects, which contain the pointer to the cv::Mat
            // data and the memory layout information (strides, width, height).
            auto inBuf = halideWrapper.wrap<uint8_t>(srcImg);
            auto outBuf = halideWrapper.wrap<uint8_t>(dstImg);
            std::cerr << "[Pipeline debug] Buffers wrapped" << std::endl;

            // 6.2.2 Set Input
            m_pipelineCache.inputParam.set(inBuf);

            // 6.2.3 Set Input Buffer as Dirty
            // CRITICAL: Mark the input buffer as dirty on the host.
            // This is necessary to inform Halide that the host-side (CPU) data (image)
            // has been modified and needs to be synchronized with the device (e.g., GPU)
            // before pipeline execution, ensuring the latest image data is processed.
            inBuf.set_host_dirty();

            // 6.2.4 Run Pipeline
            // output buffer is the destination image
            // target is the device found by HalideWrapper
            std::cerr << "[Pipeline debug] About to realize. Target: " << halideWrapper.getTarget().to_string() << std::endl;
            m_pipelineCache.pipeline.realize(outBuf, halideWrapper.getTarget());
            std::cerr << "[Pipeline debug] Realize finished" << std::endl;

            // 6.2.5 Copy Output Buffer to Host
            // CRITICAL: Copy the output buffer back to the host (CPU) memory.
            // This is necessary when using GPU acceleration, as the output buffer is stored on the
            // device. Copying back ensures the processed image data is available in the host memory
            // for further use.
            if (halideWrapper.isGPU()) {
                outBuf.copy_to_host();  // Copy the output buffer back to the host (CPU) memory.
                outBuf.device_sync();   // Stop CPU until the device (e.g., GPU) is done with
                                        // transfering the data.
            }
        }

        // 6.3. 16-bit Image
        // Same as above but for 16-bit images
        else {
            std::cerr << "[Pipeline debug] 6.3. 16-bit Image branch entered" << std::endl;
            auto inBuf = halideWrapper.wrap<uint16_t>(srcImg);
            auto outBuf = halideWrapper.wrap<uint16_t>(dstImg);

            m_pipelineCache.inputParam.set(inBuf);

            inBuf.set_host_dirty();

            std::cerr << "[Pipeline debug] About to realize (16-bit). Target: " << halideWrapper.getTarget().to_string() << std::endl;
            m_pipelineCache.pipeline.realize(outBuf, halideWrapper.getTarget());
            std::cerr << "[Pipeline debug] Realize finished" << std::endl;

            if (halideWrapper.isGPU()) {
                outBuf.copy_to_host();
                outBuf.device_sync();
            }
        }

        return dstImg;

    } catch (const Halide::Error& e) {
        std::cerr << "[Pipeline] Halide Implementation Error: " << e.what() << std::endl;
        // Invalidate cache on error to force rebuild next time
        m_pipelineCache.ops.clear();
        return srcImg;
    } catch (const std::exception& e) {
        std::cerr << "[Pipeline] Fused Runtime Error: " << e.what() << std::endl;
        m_pipelineCache.ops.clear();
        return srcImg;
    }
}

cv::Mat ImagePipeline::processUpTo(int operationIndex) {
    if (m_originalImg.empty()) {
        std::cerr << "Error: Cannot process pipeline - no image loaded\n";
        return cv::Mat();
    }

    if (operationIndex < 0 || operationIndex > static_cast<int>(m_operations.size())) {
        std::cerr << "Error: Invalid operation index for partial processing: " << operationIndex
                  << "\n";
        return process();
    }

    cv::Mat result = m_originalImg.clone();

    try {
        for (int i = 0; i < operationIndex; i++) {
            auto& operation = m_operations[i];
            if (!operation) {
                std::cerr << "Warning: Null operation at index " << i << ", skipping\n";
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
        std::cerr << "Error during partial pipeline processing: " << e.what() << "\n";
        return m_originalImg.clone();
    }

    return result;
}

// --- UNDO SYSTEM ---

void ImagePipeline::undo() {
    // Check if there is anything to undo
    if (m_undoStack.empty()) {
        std::cout << "[ImagePipeline] Nothing to undo\n";
        return;
    }

    // 1. Get last action from undo stack and take it out of the undo stack
    PipelineAction action = m_undoStack.back();
    m_undoStack.pop_back();

    // 2. Revert logic
    switch (action.type) {
        case PipelineAction::INSERT:
            // Reverse: Remove at index
            if (action.index < (int)m_operations.size()) {
                m_operations.erase(m_operations.begin() + action.index);
                std::cout << "[Undo] Reverted Insert at " << action.index << "\n";
            }
            break;

        case PipelineAction::REMOVE:
            // Reverse: Insert at index
            m_operations.insert(m_operations.begin() + action.index, action.operation);
            std::cout << "[Undo] Restored " << action.operation->getName() << " at " << action.index
                      << "\n";
            break;

        case PipelineAction::MODIFY:
            // Reverse: Restore oldOp
            if (action.index < (int)m_operations.size()) {
                m_operations[action.index] = action.oldOperation;
                std::cout << "[Undo] Reverted Modify at " << action.index << "\n";
            }
            break;
    }

    // 3. Move to Redo Stack
    m_redoStack.push_back(action);

    // 4. Invalidate cache to force reprocessing
    invalidateCache();
}

void ImagePipeline::redo() {
    if (m_redoStack.empty()) {
        std::cout << "[ImagePipeline] Nothing to redo\n";
        return;
    }

    // 1. Get last action
    PipelineAction action = m_redoStack.back();
    m_redoStack.pop_back();

    // 2. Re-Apply logic
    switch (action.type) {
        case PipelineAction::INSERT:
            // Redo: Insert at index
            m_operations.insert(m_operations.begin() + action.index, action.operation);
            std::cout << "[Redo] Re-Insert at " << action.index << "\n";
            break;

        case PipelineAction::REMOVE:
            // Redo: Remove at index
            if (action.index < (int)m_operations.size()) {
                m_operations.erase(m_operations.begin() + action.index);
                std::cout << "[Redo] Re-Remove at " << action.index << "\n";
            }
            break;

        case PipelineAction::MODIFY:
            // Redo: Restore newOp
            if (action.index < (int)m_operations.size()) {
                m_operations[action.index] = action.operation;
                std::cout << "[Redo] Re-Modify at " << action.index << "\n";
            }
            break;
    }

    // 3. Move to Undo Stack
    m_undoStack.push_back(action);

    invalidateCache();
}

void ImagePipeline::invalidateCache() {
    // 1. Invalidate cache to force reprocessing
    m_cacheValid = false;

    // 2. Release memory of cached result
    m_cachedResult.release();
}

void ImagePipeline::updateCache(const cv::Mat& result) {
    if (!result.empty()) {
        // 1. Update cache
        m_cachedResult = result.clone();

        // 2. Mark cache as valid
        m_cacheValid = true;
    }
}

// --- THIS IS FOR PRESET SAVING AND LOADING ---
// WILL BE IMPLEMENTED LATER

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