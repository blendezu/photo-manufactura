#ifndef IMAGE_PIPELINE_H
#define IMAGE_PIPELINE_H

#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#include "../core/operation_base.h"

class ImagePipeline {
   private:
    cv::Mat originalImg;
    std::vector<std::shared_ptr<ImageOperation>> operations;
    std::vector<std::shared_ptr<ImageOperation>> undoneOperations;
    cv::Mat cachedResult;
    bool cacheValid;
    std::shared_ptr<ImageOperation> liveOperation;

   public:
    ImagePipeline();

    // set and manage the image
    void setImg(const cv::Mat& img);
    cv::Mat getOriginalImg() const {
        return originalImg.clone();
    }
    bool hasImg() const {
        return !originalImg.empty();
    }

    // operations management
    void addOperation(std::shared_ptr<ImageOperation> operation);
    void insertOperation(int index, std::shared_ptr<ImageOperation>);
    void removeOperation(int index);
    void clearOperations();
    void clearUndoHistory();

    // call the operation
    size_t getOperationCount() const {
        return operations.size();
    }
    std::shared_ptr<ImageOperation> getOperation(int index);
    const std::vector<std::shared_ptr<ImageOperation>>& getOperations() const {
        return operations;
    }

    // Live-Operation for realtime preview
    void setLiveOperation(std::shared_ptr<ImageOperation> operation);
    void clearLiveOperation();

    // Pipeline
    cv::Mat process();
    cv::Mat processUpTo(int operationIndex);

    // Undo/Redo
    void undo();
    void redo();
    bool canUndo() const {
        return !operations.empty();
    }
    bool canRedo() const {
        return !undoneOperations.empty();
    }
    size_t getUndoCount() const {
        return undoneOperations.size();
    }

    // Cache management
    void invalidateCache();
    bool isCacheValid() const {
        return cacheValid;
    }

    std::string serializePipeline() const;
    void deserialziePipeline(const std::string& data);

   private:
    void updateCache(const cv::Mat& result);
};

#endif