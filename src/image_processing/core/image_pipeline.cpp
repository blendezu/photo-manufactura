#include "image_pipeline.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>

ImagePipeline::ImagePipeline() : cacheValid(false) {}

void ImagePipeline::setImg(const cv::Mat& img) {
    if (img.empty()) {
        std::cerr << "Warning: Setting empty image to pipeline\n";
        return;
    }

    originalImg = img.clone();
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

    operations.push_back(operation);
    undoneOperations.clear();  // delete Redo history when new operation
    invalidateCache();

    std::cout << "Operation added: " << operation->getName() << " (Total: " << operations.size()
              << ")" << std::endl;
}

void ImagePipeline::insertOperation(int index, std::shared_ptr<ImageOperation> operation) {
    if (!operation) {
        std::cerr << "Error: Cannot insert null operation\n";
        return;
    }

    if (index < 0 || index > static_cast<int>(operations.size())) {
        std::cerr << "Error: Invalid index for operation insertion: " << index << std::endl;
        return;
    }

    operations.insert(operations.begin() + index, operation);
    undoneOperations.clear();
    invalidateCache();

    std::cout << "Operation inserted at " << index << ": " << operation->getName() << std::endl;
}

void ImagePipeline::removeOperation(int index) {
    if (index < 0 || index >= static_cast<int>(operations.size())) {
        std::cerr << "Error: Invalid index for operation removal: " << index << std::endl;
        return;
    }

    std::string opName = operations[index]->getName();
    operations.erase(operations.begin() + index);
    invalidateCache();

    std::cout << "Operation removed at " << index << ": " << opName << std::endl;
}

void ImagePipeline::clearOperations() {
    if (!operations.empty()) {
        std::cout << "Clearing all operations (" << operations.size() << " total)" << std::endl;
        operations.clear();
        invalidateCache();
    }
}

void ImagePipeline::clearUndoHistory() {
    if (!undoneOperations.empty()) {
        undoneOperations.clear();
        std::cout << "Undo history cleared" << std::endl;
    }
}

std::shared_ptr<ImageOperation> ImagePipeline::getOperation(int index) {
    if (index < 0 || index >= static_cast<int>(operations.size())) {
        std::cerr << "Error: Invalid operation index: " << index << std::endl;
        return nullptr;
    }

    return operations[index];
}

void ImagePipeline::setLiveOperation(std::shared_ptr<ImageOperation> operation) {
    liveOperation = operation;
}

void ImagePipeline::clearLiveOperations() {
    liveOperation = nullptr;
}

cv::Mat ImagePipeline::process() {
    if (originalImg.empty()) {
        std::cerr << "Error: Cannot process pipeline - no image loaded" << std::endl;
        return cv::Mat();
    }

    // ✅ Cache für normale Operationen (ohne Live-Op)
    if (cacheValid && !cachedResult.empty() && !liveOperation) {
        return cachedResult.clone();
    }

    // ✅ OPTIMIERT: Cache für Vorschau (mit Live-Op) - NUR Live-Op wird berechnet!
    if (cacheValid && !cachedResult.empty() && liveOperation) {
        cv::Mat result = cachedResult.clone();

        cv::Mat previous = result.clone();
        result = liveOperation->apply(result);

        if (result.empty()) {
            std::cerr << "Error: Live operation returned empty image" << std::endl;
            return previous;
        }
        return result;  // ⚡ Nur Live-Op auf Cache angewendet!
    }

    // ❌ Cache ungültig - normale Berechnung
    cv::Mat result = originalImg.clone();

    try {
        for (size_t i = 0; i < operations.size(); i++) {
            auto& operation = operations[i];
            if (!operation) {
                std::cerr << "Warning: Null operation at index " << i << ", skipping" << std::endl;
                continue;
            }

            cv::Mat previous = result.clone();
            result = operation->apply(result);

            if (result.empty()) {
                std::cerr << "Error: Operation " << operation->getName() << " at index " << i
                          << " returned empty image" << std::endl;
                result = previous;  // back to the last one
                break;
            }
        }

        // Live-Operation apply if active
        if (liveOperation) {
            cv::Mat previous = result.clone();
            result = liveOperation->apply(result);

            if (result.empty()) {
                std::cerr << "Error: Live operation returned empty image" << std::endl;
                result = previous;
            }
        }

        // Cache update
        updateCache(result);
    } catch (const std::exception& e) {
        std::cerr << "Error during pipeline processing: " << e.what() << std::endl;
        invalidateCache();
        return originalImg.clone();  // fallback to original
    }

    return result;
}

cv::Mat ImagePipeline::processUpTo(int operationIndex) {
    if (originalImg.empty()) {
        std::cerr << "Error: Cannot process pipeline - no image loaded\n";
        return cv::Mat();
    }

    if (operationIndex < 0 || operationIndex > static_cast<int>(operations.size())) {
        std::cerr << "Error: Invalid operation index for partial processing: " << operationIndex
                  << std::endl;
        return process();
    }

    cv::Mat result = originalImg.clone();

    try {
        for (int i = 0; i < operationIndex; i++) {
            auto& operation = operations[i];
            if (!operation) {
                std::cerr << "Warning: Null operation at index " << i << ", skipping" << std::endl;
                continue;
            }

            result = operation->apply(result);

            if (result.empty()) {
                std::cerr << "Error: Operation " << operation->getName() << " at index " << i
                          << "returned empty image\n";
                return originalImg.clone();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during partial pipeline processing: " << e.what() << std::endl;
        return originalImg.clone();
    }

    return result;
}

void ImagePipeline::undo() {
    if (operations.empty()) {
        std::cout << "Nothing to undo\n";
        return;
    }

    auto lastOp = operations.back();
    undoneOperations.push_back(lastOp);
    operations.pop_back();
    invalidateCache();

    std::cout << "Undo: " << lastOp->getName() << std::endl;
}

void ImagePipeline::redo() {
    if (undoneOperations.empty()) {
        std::cout << "Nothing to redo" << std::endl;
        return;
    }

    auto lastUndone = undoneOperations.back();
    operations.push_back(lastUndone);
    undoneOperations.pop_back();
    invalidateCache();

    std::cout << "Redo: " << lastUndone->getName() << std::endl;
}

void ImagePipeline::invalidateCache() {
    cacheValid = false;
    cachedResult.release();
}

void ImagePipeline::updateCache(const cv::Mat& result) {
    if (!result.empty()) {
        cachedResult = result.clone();
        cacheValid = true;
    }
}

std::string ImagePipeline::serializePipeline() const {
    std::stringstream ss;
    ss << "ImagePipeline v1.0|";
    ss << "Operations:" << operations.size() << "|";

    for (size_t i = 0; i < operations.size(); i++) {
        if (operations[i]) {
            ss << operations[i]->getName() << ":" << operations[i]->getSettings();
            if (i < operations.size() - 1) {
                ss << ";";
            }
        }
    }

    return ss.str();
}

void ImagePipeline::deserialziePipeline(const std::string& data) {
    std::cout << "Deserializing pipeline: " << data.substr(0, 50) << "..." << std::endl;
}