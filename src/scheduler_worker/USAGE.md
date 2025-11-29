# Scheduler Worker Module - Usage Examples

## Basic Usage

### 1. Simple Function Task

```cpp
#include "TaskScheduler.h"

void executeSimpleTask() {
    TaskScheduler& scheduler = TaskScheduler::instance();
    
    // Submit a simple lambda task
    int taskId = scheduler.submitTask([]() {
        qDebug() << "Task executing on worker thread!";
        QThread::msleep(1000);  // Simulate work
        qDebug() << "Task completed!";
    }, TaskPriority::NORMAL);
    
    qDebug() << "Task submitted with ID:" << taskId;
}
```

### 2. Task with Result Handling

```cpp
#include "TaskScheduler.h"
#include <QObject>

void taskWithResultHandling(QObject* receiver) {
    TaskScheduler& scheduler = TaskScheduler::instance();
    
    // Connect to completion signal
    QObject::connect(&scheduler, &TaskScheduler::taskCompleted,
                     receiver, [](int taskId, QVariant result) {
        qDebug() << "Task" << taskId << "completed with result:" << result;
    });
    
    // Submit task
    scheduler.submitTask([]() {
        // Perform computation
        return 42;
    });
}
```

### 3. Progress Reporting

```cpp
void taskWithProgress(QObject* receiver) {
    TaskScheduler& scheduler = TaskScheduler::instance();
    
    // Connect to progress signal
    QObject::connect(&scheduler, &TaskScheduler::taskProgress,
                     receiver, [](int taskId, int percent) {
        qDebug() << "Task" << taskId << "progress:" << percent << "%";
        // Update progress bar in UI
    });
    
    // Submit task
    scheduler.submitTask([]() {
        // Task implementation emits progress
    });
}
```

## Image Processing Integration

### Integration with DocumentManager (Asynchronous)

Add this to `src/model/DocumentManager.h`:

```cpp
// In DocumentManager class
public slots:
    void applyAdjustmentsAsync();  // New async version
    
private slots:
    void onImageProcessingComplete(int taskId, QVariant result);
    void onImageProcessingProgress(int taskId, int percent);
    void onImageProcessingFailed(int taskId, QString error);
    
private:
    int m_currentTaskId = -1;
```

Add this to `src/model/DocumentManager.cpp`:

```cpp
#include "TaskScheduler.h"
#include "ImageProcessingTask.h"

// In constructor, connect scheduler signals
DocumentManager::DocumentManager(QObject* parent)
    : QObject(parent),
      m_currentDocument(std::make_unique<ImageDocument>(this)),
      m_adjustments(std::make_unique<AdjustmentSettings>(this)),
      m_imagePipeline(std::make_unique<ImagePipeline>()) {
    
    // ... existing code ...
    
    // Connect scheduler signals
    TaskScheduler& scheduler = TaskScheduler::instance();
    connect(&scheduler, &TaskScheduler::taskCompleted,
            this, &DocumentManager::onImageProcessingComplete);
    connect(&scheduler, &TaskScheduler::taskProgress,
            this, &DocumentManager::onImageProcessingProgress);
    connect(&scheduler, &TaskScheduler::taskFailed,
            this, &DocumentManager::onImageProcessingFailed);
}

void DocumentManager::applyAdjustmentsAsync() {
    if (!hasDocument()) {
        return;
    }
    
    // Cancel previous task if still running
    if (m_currentTaskId >= 0) {
        TaskScheduler::instance().cancelTask(m_currentTaskId);
    }
    
    // Build operations list
    std::vector<std::shared_ptr<ImageOperation>> operations;
    
    if (m_adjustments->brightness() != 0) {
        operations.push_back(std::make_shared<AdjustBrightness>(
            m_adjustments->brightness()));
    }
    if (m_adjustments->contrast() != 0) {
        operations.push_back(std::make_shared<AdjustContrast>(
            m_adjustments->contrast()));
    }
    // ... add other operations ...
    
    // Get original image as cv::Mat
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->originalImage());
    
    // Submit async task
    m_currentTaskId = TaskScheduler::instance().submitImageProcessingTask(
        cvImage, operations, TaskPriority::NORMAL);
    
    qDebug() << "Submitted async image processing task:" << m_currentTaskId;
}

void DocumentManager::onImageProcessingComplete(int taskId, QVariant result) {
    if (taskId != m_currentTaskId) {
        return;  // Not our task
    }
    
    // Extract result
    cv::Mat resultMat = result.value<cv::Mat>();
    if (resultMat.empty()) {
        qWarning() << "Image processing returned empty result";
        return;
    }
    
    // Convert back to QImage
    QImage processedQImage = cvMatToQImage(resultMat);
    
    // Update document
    m_currentDocument->setProcessedImage(processedQImage);
    m_currentDocument->setModified(true);
    
    m_currentTaskId = -1;
    
    qDebug() << "Image processing completed successfully";
}

void DocumentManager::onImageProcessingProgress(int taskId, int percent) {
    if (taskId != m_currentTaskId) {
        return;
    }
    
    qDebug() << "Processing progress:" << percent << "%";
    // Emit signal for UI progress bar
    emit processingProgress(percent);
}

void DocumentManager::onImageProcessingFailed(int taskId, QString error) {
    if (taskId != m_currentTaskId) {
        return;
    }
    
    qWarning() << "Image processing failed:" << error;
    emit errorOccurred(QString("Image processing failed: %1").arg(error));
    
    m_currentTaskId = -1;
}
```

### Update Controller to use Async Processing

In `src/controller/ApplicationController.cpp`:

```cpp
void ApplicationController::adjustBrightness(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setBrightness(value);
        setState("isModified", true);
        
        // Use async processing instead of sync
        m_documentManager->applyAdjustmentsAsync();
    }
}

// Apply to all adjustment methods:
// adjustContrast, adjustExposure, adjustHighlights, etc.
```

## Advanced Usage

### 1. Task Cancellation

```cpp
int taskId = scheduler.submitTask([]() {
    for (int i = 0; i < 100; ++i) {
        // Check cancellation periodically
        if (/* task cancelled */) {
            return;
        }
        // Do work
        QThread::msleep(10);
    }
});

// Cancel from UI
scheduler.cancelTask(taskId);
```

### 2. Priority Tasks

```cpp
// High priority (UI-blocking operations)
scheduler.submitTask([]() {
    // Critical operation
}, TaskPriority::HIGH);

// Normal priority (default)
scheduler.submitTask([]() {
    // Regular adjustment
}, TaskPriority::NORMAL);

// Low priority (background work)
scheduler.submitTask([]() {
    // Batch processing
}, TaskPriority::LOW);
```

### 3. Batch Processing Multiple Images

```cpp
void processBatchImages(const QStringList& filePaths) {
    TaskScheduler& scheduler = TaskScheduler::instance();
    
    for (const QString& path : filePaths) {
        scheduler.submitTask([path]() {
            // Load image
            QImage image(path);
            
            // Process
            cv::Mat cvImage = qImageToCvMat(image);
            // ... apply operations ...
            
            // Save result
            QString outputPath = path + "_processed.jpg";
            result.save(outputPath);
            
            qDebug() << "Processed:" << path;
        }, TaskPriority::LOW);  // Low priority for batch
    }
}
```

### 4. Thread Pool Configuration

```cpp
// Get current settings
int threadCount = scheduler.maxThreads();
int activeThreads = scheduler.activeThreadCount();

qDebug() << "Thread pool:" << activeThreads << "/" << threadCount;

// Adjust thread count based on workload
scheduler.setMaxThreads(8);  // Use 8 worker threads

// Pause scheduler during heavy operations
scheduler.pauseScheduler();
// ... do something ...
scheduler.resumeScheduler();
```

### 5. Wait for Completion

```cpp
// Submit tasks
for (int i = 0; i < 10; ++i) {
    scheduler.submitTask([]() { /* work */ });
}

// Wait for all to complete (5 second timeout)
if (scheduler.waitForAll(5000)) {
    qDebug() << "All tasks completed";
} else {
    qDebug() << "Timeout - some tasks still running";
}
```

## Error Handling

### Handling Task Failures

```cpp
TaskScheduler& scheduler = TaskScheduler::instance();

connect(&scheduler, &TaskScheduler::taskFailed,
        this, [](int taskId, QString error) {
    qWarning() << "Task" << taskId << "failed:" << error;
    
    // Show error dialog to user
    QMessageBox::warning(nullptr, "Task Failed", 
                        QString("Task %1 failed: %2")
                        .arg(taskId).arg(error));
});

// Submit task that might fail
scheduler.submitTask([]() {
    throw std::runtime_error("Something went wrong!");
});
```

## Performance Tips

1. **Don't submit too many small tasks** - overhead of threading
   ```cpp
   // BAD: 1000 tiny tasks
   for (int i = 0; i < 1000; ++i) {
       scheduler.submitTask([i]() { process(i); });
   }
   
   // GOOD: Batch into larger chunks
   int batchSize = 100;
   for (int i = 0; i < 10; ++i) {
       scheduler.submitTask([i, batchSize]() {
           for (int j = 0; j < batchSize; ++j) {
               process(i * batchSize + j);
           }
       });
   }
   ```

2. **Clone data before passing to tasks** - avoid race conditions
   ```cpp
   cv::Mat image = getImage();
   
   // GOOD: Clone for thread safety
   scheduler.submitImageProcessingTask(image.clone(), operations);
   ```

3. **Use appropriate priorities**
   - HIGH: User-visible operations (< 100ms)
   - NORMAL: Regular adjustments (100ms - 1s)
   - LOW: Background/batch (> 1s, not user-initiated)

## Debugging

### Enable Debug Output

```cpp
// In main.cpp
QLoggingCategory::setFilterRules("*.debug=true");

// Or set environment variable
// QT_LOGGING_RULES="*.debug=true"
```

### Monitor Active Tasks

```cpp
void monitorScheduler() {
    TaskScheduler& scheduler = TaskScheduler::instance();
    
    qDebug() << "Active threads:" << scheduler.activeThreadCount();
    qDebug() << "Max threads:" << scheduler.maxThreads();
    qDebug() << "Paused:" << scheduler.isPaused();
}
```

## Migration from Sync to Async

### Before (Synchronous)
```cpp
void DocumentManager::applyAdjustments() {
    cv::Mat result = m_imagePipeline->process();
    // UI freezes during processing
    updateImage(result);
}
```

### After (Asynchronous)
```cpp
void DocumentManager::applyAdjustmentsAsync() {
    TaskScheduler::instance().submitImageProcessingTask(
        m_image, m_operations);
    // UI remains responsive
}

void DocumentManager::onComplete(int id, QVariant result) {
    updateImage(result.value<cv::Mat>());
    // Called when processing finishes
}
```

## Testing

Run the unit tests:
```bash
cd build
ctest -R SchedulerWorkerTest -V
```

Or directly:
```bash
./build/bin/scheduler_worker_test
```
