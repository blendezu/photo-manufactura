# Scheduler Worker Module

## Status: Not Implemented (Planned)

This module is a placeholder for future asynchronous task scheduling and worker thread management.

---

## Purpose

Provides asynchronous task scheduling and worker thread management for non-blocking image processing operations.

### Why It's Needed

Currently, all image processing happens **synchronously** on the main thread:
- UI can freeze during heavy operations (large images, multiple adjustments)
- No progress feedback for long-running tasks
- Cannot cancel ongoing operations
- Batch processing would block the entire application

---

## Planned Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Main Thread (UI)                     │
│  ┌────────────┐         signals          ┌───────────┐ │
│  │ Controller │ ──────────────────────▶  │ Scheduler │ │
│  └────────────┘                          └─────┬─────┘ │
└──────────────────────────────────────────────┼─────────┘
                                               │
                    ┌──────────────────────────┼──────────────────────────┐
                    │                          ▼                          │
                    │              ┌────────────────────┐                 │
                    │              │    Task Queue      │                 │
                    │              │ (Priority-based)   │                 │
                    │              └────────────────────┘                 │
                    │                          │                          │
                    │        ┌─────────────────┼─────────────────┐       │
                    │        ▼                 ▼                 ▼       │
                    │  ┌──────────┐     ┌──────────┐     ┌──────────┐  │
                    │  │ Worker 1 │     │ Worker 2 │     │ Worker 3 │  │
                    │  │ (QThread)│     │ (QThread)│     │ (QThread)│  │
                    │  └──────────┘     └──────────┘     └──────────┘  │
                    │        │                 │                 │       │
                    │        └─────────────────┴─────────────────┘       │
                    │                          │                          │
                    │                          ▼                          │
                    │              ┌────────────────────┐                 │
                    │              │  Image Processing  │                 │
                    │              │   (ImagePipeline)  │                 │
                    │              └────────────────────┘                 │
                    │                          │                          │
                    └──────────────────────────┼──────────────────────────┘
                                               │ emit signals
                    ┌──────────────────────────▼──────────────────────────┐
                    │            Main Thread (Update UI)                  │
                    └─────────────────────────────────────────────────────┘
```

---

## Planned Features

### Phase 1: Basic Task Scheduling
- [ ] Task abstraction (`Task` base class)
- [ ] Task queue with FIFO ordering
- [ ] Single worker thread (QThread-based)
- [ ] Signal-based result delivery to main thread

### Phase 2: Advanced Features
- [ ] Priority queue (HIGH, NORMAL, LOW)
- [ ] Thread pool with configurable size
- [ ] Task cancellation support
- [ ] Progress reporting (0-100%)

### Phase 3: Optimization
- [ ] Task result caching
- [ ] Smart thread allocation based on CPU cores
- [ ] Task grouping/batching
- [ ] Automatic retry on failure

---

## Proposed API

### TaskScheduler (Singleton)

```cpp
class TaskScheduler : public QObject {
    Q_OBJECT
    
public:
    static TaskScheduler& instance();
    
    // Submit tasks
    int submitTask(std::function<void()> task, 
                   TaskPriority priority = TaskPriority::NORMAL);
    
    int submitImageProcessingTask(
        cv::Mat image,
        std::vector<std::shared_ptr<ImageOperation>> operations,
        TaskPriority priority = TaskPriority::NORMAL
    );
    
    // Task control
    void cancelTask(int taskId);
    void cancelAllTasks();
    void pauseScheduler();
    void resumeScheduler();
    
    // Configuration
    void setMaxThreads(int count);
    int maxThreads() const;
    
signals:
    void taskStarted(int taskId);
    void taskProgress(int taskId, int percent);
    void taskCompleted(int taskId, QVariant result);
    void taskFailed(int taskId, QString error);
    void taskCancelled(int taskId);
    
private:
    TaskScheduler();
    ~TaskScheduler();
    
    QThreadPool* m_threadPool;
    QQueue<Task*> m_taskQueue;
    QMutex m_queueMutex;
    std::atomic<int> m_nextTaskId;
};
```

### Task Base Class

```cpp
class Task : public QObject, public QRunnable {
    Q_OBJECT
    
public:
    Task(int id, TaskPriority priority = TaskPriority::NORMAL);
    virtual ~Task() = default;
    
    // QRunnable interface
    void run() override;
    
    // Task control
    void cancel();
    bool isCancelled() const;
    
    // Task info
    int id() const { return m_id; }
    TaskPriority priority() const { return m_priority; }
    
signals:
    void progress(int percent);
    void completed(QVariant result);
    void failed(QString error);
    
protected:
    virtual void execute() = 0;  // Implement in subclasses
    
    std::atomic<bool> m_cancelled;
    int m_id;
    TaskPriority m_priority;
};
```

### ImageProcessingTask

```cpp
class ImageProcessingTask : public Task {
    Q_OBJECT
    
public:
    ImageProcessingTask(
        int id,
        cv::Mat image,
        std::vector<std::shared_ptr<ImageOperation>> operations
    );
    
protected:
    void execute() override;
    
private:
    cv::Mat m_image;
    std::vector<std::shared_ptr<ImageOperation>> m_operations;
};
```

---

## Integration Points

### DocumentManager (Async Processing)

```cpp
// Current (synchronous)
void DocumentManager::applyAdjustments() {
    cv::Mat result = m_imagePipeline->process();
    QImage qImage = cvMatToQImage(result);
    m_currentDocument->setProcessedImage(qImage);
}

// Future (asynchronous)
void DocumentManager::applyAdjustmentsAsync() {
    int taskId = TaskScheduler::instance().submitImageProcessingTask(
        m_imagePipeline->getOriginalImg(),
        m_imagePipeline->getOperations(),
        TaskPriority::NORMAL
    );
    
    connect(&TaskScheduler::instance(), &TaskScheduler::taskCompleted,
            this, [this](int id, QVariant result) {
        cv::Mat processedMat = result.value<cv::Mat>();
        QImage qImage = cvMatToQImage(processedMat);
        m_currentDocument->setProcessedImage(qImage);
    });
}
```

### Controller (with Progress Reporting)

```cpp
void ApplicationController::adjustBrightness(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setBrightness(value);
        
        // Show progress bar
        emit processingStarted();
        
        // Submit async task
        m_documentManager->applyAdjustmentsAsync();
        
        // Connect progress
        connect(&TaskScheduler::instance(), &TaskScheduler::taskProgress,
                this, [this](int taskId, int percent) {
            emit processingProgress(percent);
        });
        
        connect(&TaskScheduler::instance(), &TaskScheduler::taskCompleted,
                this, [this](int taskId, QVariant result) {
            emit processingComplete();
        });
    }
}
```

---

## Dependencies

### Required
- **Qt6::Core** - QThread, QThreadPool, QRunnable, QMutex
- **Qt6::Concurrent** - QtConcurrent::run() (optional alternative)
- **C++20** - std::function, std::atomic, std::shared_ptr

### Optional
- **TBB (Intel Threading Building Blocks)** - Advanced task scheduling
- **OpenMP** - Already used for intra-operation parallelism

---

## Implementation Checklist

### Step 1: Basic Structure
- [ ] Create `Task.h` and `Task.cpp` (base class)
- [ ] Create `TaskScheduler.h` and `TaskScheduler.cpp` (singleton)
- [ ] Implement basic FIFO queue
- [ ] Create simple test with dummy tasks

### Step 2: Image Processing Integration
- [ ] Create `ImageProcessingTask.h/.cpp`
- [ ] Integrate with existing `ImagePipeline`
- [ ] Handle cv::Mat thread-safe cloning
- [ ] Test with single adjustment operation

### Step 3: Signal/Slot Wiring
- [ ] Connect scheduler signals to controller
- [ ] Update UI to show "Processing..." indicator
- [ ] Test with multiple rapid adjustments

### Step 4: Advanced Features
- [ ] Implement priority queue
- [ ] Add task cancellation
- [ ] Add progress reporting (ImagePipeline needs to emit progress)
- [ ] Add thread pool configuration

### Step 5: Error Handling
- [ ] Handle pipeline errors in worker threads
- [ ] Emit error signals to UI
- [ ] Graceful degradation to synchronous mode if threading fails

---

## Thread Safety Considerations

### Safe Operations
✅ Reading immutable data (originalImage)  
✅ Cloning cv::Mat before processing  
✅ Qt signal/slot cross-thread communication (queued connections)  

### Unsafe Operations (Avoid!)
❌ Modifying shared state from worker threads  
❌ Direct UI updates from worker threads  
❌ Accessing QWidget/QImage without proper synchronization  

### Thread-Safe Pattern
```cpp
void ImageProcessingTask::execute() {
    // 1. Clone data (thread-safe)
    cv::Mat imageCopy = m_image.clone();
    
    // 2. Process (isolated, no shared state)
    ImagePipeline pipeline;
    pipeline.setImg(imageCopy);
    for (auto& op : m_operations) {
        pipeline.addOperation(op);
    }
    cv::Mat result = pipeline.process();
    
    // 3. Signal back to main thread (queued connection)
    emit completed(QVariant::fromValue(result));
}
```

---

## Performance Targets

| Operation | Current (Sync) | Target (Async) |
|-----------|----------------|----------------|
| Single adjustment | ~50ms | Non-blocking |
| 10 adjustments | ~500ms | Non-blocking |
| 4K image processing | ~2s | Background + progress bar |
| Batch (10 images) | Not supported | Parallel processing |

---

## Testing Strategy

### Unit Tests
```cpp
TEST(TaskSchedulerTest, SubmitTask) {
    TaskScheduler& scheduler = TaskScheduler::instance();
    bool executed = false;
    
    scheduler.submitTask([&executed]() {
        executed = true;
    });
    
    QTest::qWait(100);  // Wait for async execution
    ASSERT_TRUE(executed);
}
```

### Integration Tests
```cpp
TEST(IntegrationTest, AsyncImageProcessing) {
    DocumentManager docMgr;
    docMgr.openDocument("test.jpg");
    
    QSignalSpy spy(&TaskScheduler::instance(), 
                   &TaskScheduler::taskCompleted);
    
    docMgr.applyAdjustmentsAsync();
    
    ASSERT_TRUE(spy.wait(5000));  // 5s timeout
}
```

---

## Future Enhancements

- **GPU Task Scheduling** - Coordinate CPU and GPU tasks
- **Distributed Processing** - Network-based task distribution
- **Smart Caching** - Cache-aware task scheduling
- **Load Balancing** - Dynamic thread allocation based on system load
- **Machine Learning Integration** - Schedule AI-based enhancements

---

## References

- [Qt Threading Basics](https://doc.qt.io/qt-6/threads.html)
- [QThreadPool Documentation](https://doc.qt.io/qt-6/qthreadpool.html)
- [Qt Concurrent Framework](https://doc.qt.io/qt-6/qtconcurrent-index.html)
- Intel TBB: https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html

---

**Last Updated:** 29 November 2025  
**Status:** Planning phase - Ready for implementation when needed
