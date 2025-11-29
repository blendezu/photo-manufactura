#pragma once

#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QThreadPool>
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>

#include "Task.h"

// Forward declarations
namespace cv {
class Mat;
}
class ImageOperation;

/**
 * @brief Singleton task scheduler for managing asynchronous operations
 *
 * Manages a thread pool and priority-based task queue for non-blocking
 * execution of image processing and other long-running operations.
 *
 * Thread Safety: All public methods are thread-safe.
 *
 * Usage Example:
 * @code
 * TaskScheduler& scheduler = TaskScheduler::instance();
 * int taskId = scheduler.submitTask([]() {
 *     // Long-running operation
 * }, TaskPriority::NORMAL);
 *
 * connect(&scheduler, &TaskScheduler::taskCompleted,
 *         this, [](int id, QVariant result) {
 *     qDebug() << "Task" << id << "completed";
 * });
 * @endcode
 */
class TaskScheduler : public QObject {
    Q_OBJECT

   public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the global TaskScheduler
     */
    static TaskScheduler& instance();

    /**
     * @brief Submit a function for asynchronous execution
     * @param function Function to execute on worker thread
     * @param priority Task priority (affects scheduling order)
     * @return Unique task ID for tracking
     */
    int submitTask(std::function<void()> function, TaskPriority priority = TaskPriority::NORMAL);

    /**
     * @brief Submit an image processing task
     * @param image Source image to process
     * @param operations List of operations to apply
     * @param priority Task priority
     * @return Unique task ID
     */
    int submitImageProcessingTask(const cv::Mat& image,
                                  const std::vector<std::shared_ptr<ImageOperation>>& operations,
                                  TaskPriority priority = TaskPriority::NORMAL);

    /**
     * @brief Cancel a specific task
     * @param taskId Task ID to cancel
     *
     * If task is already executing, sets cancellation flag.
     * If task is queued, removes from queue.
     */
    void cancelTask(int taskId);

    /**
     * @brief Cancel all pending and running tasks
     */
    void cancelAllTasks();

    /**
     * @brief Pause task scheduler (stops accepting new tasks)
     */
    void pauseScheduler();

    /**
     * @brief Resume task scheduler
     */
    void resumeScheduler();

    /**
     * @brief Check if scheduler is paused
     * @return true if paused
     */
    bool isPaused() const {
        return m_paused.load(std::memory_order_acquire);
    }

    /**
     * @brief Set maximum number of worker threads
     * @param count Thread count (1-16, default: CPU core count)
     */
    void setMaxThreads(int count);

    /**
     * @brief Get maximum thread count
     * @return Maximum number of worker threads
     */
    int maxThreads() const;

    /**
     * @brief Get number of active (running) threads
     * @return Active thread count
     */
    int activeThreadCount() const;

    /**
     * @brief Wait for all tasks to complete
     * @param timeoutMs Timeout in milliseconds (-1 for infinite)
     * @return true if all tasks completed, false if timeout
     */
    bool waitForAll(int timeoutMs = -1);

   signals:
    /**
     * @brief Emitted when a task starts execution
     * @param taskId Task ID
     */
    void taskStarted(int taskId);

    /**
     * @brief Emitted to report task progress
     * @param taskId Task ID
     * @param percent Progress percentage (0-100)
     */
    void taskProgress(int taskId, int percent);

    /**
     * @brief Emitted when a task completes successfully
     * @param taskId Task ID
     * @param result Task result
     */
    void taskCompleted(int taskId, QVariant result);

    /**
     * @brief Emitted when a task fails
     * @param taskId Task ID
     * @param error Error message
     */
    void taskFailed(int taskId, QString error);

    /**
     * @brief Emitted when a task is cancelled
     * @param taskId Task ID
     */
    void taskCancelled(int taskId);

   private:
    // Singleton pattern - private constructor/destructor
    TaskScheduler();
    ~TaskScheduler();
    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    /**
     * @brief Connect task signals to scheduler signals
     * @param task Task to connect
     */
    void connectTaskSignals(Task* task);

    /**
     * @brief Generate next unique task ID
     * @return New task ID
     */
    int nextTaskId();

    /**
     * @brief Register a task for tracking
     * @param taskId Task ID
     * @param task Task pointer
     */
    void registerTask(int taskId, Task* task);

    /**
     * @brief Unregister a task (called when task completes)
     * @param taskId Task ID
     */
    void unregisterTask(int taskId);

    // Thread pool for task execution
    QThreadPool* m_threadPool;

    // Task tracking
    std::atomic<int> m_nextTaskId;
    QMutex m_taskMapMutex;
    std::unordered_map<int, Task*> m_activeTasks;  // Currently executing tasks

    // Scheduler state
    std::atomic<bool> m_paused;
};
