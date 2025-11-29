#pragma once

#include <QObject>
#include <QRunnable>
#include <QString>
#include <QVariant>
#include <atomic>
#include <functional>

/**
 * @brief Task priority levels for scheduling
 */
enum class TaskPriority {
    HIGH = 0,    // UI-blocking operations (immediate execution)
    NORMAL = 1,  // Regular adjustments (default)
    LOW = 2      // Background/batch processing (when idle)
};

/**
 * @brief Base class for all scheduled tasks
 *
 * Provides common functionality for task execution, cancellation,
 * and progress reporting. Tasks run on worker threads via QThreadPool.
 *
 * Thread Safety:
 * - Can be created on any thread
 * - execute() runs on worker thread
 * - Signals are delivered to receiver's thread (queued connections)
 */
class Task : public QObject, public QRunnable {
    Q_OBJECT

   public:
    /**
     * @brief Construct a new Task
     * @param id Unique task identifier
     * @param priority Task priority (affects scheduling order)
     * @param parent Parent QObject (optional)
     */
    explicit Task(int id, TaskPriority priority = TaskPriority::NORMAL, QObject* parent = nullptr);

    virtual ~Task() = default;

    // QRunnable interface (called by QThreadPool)
    void run() override final;

    /**
     * @brief Request task cancellation
     *
     * Sets cancellation flag. Task implementation should check
     * isCancelled() periodically and abort gracefully.
     */
    void cancel();

    /**
     * @brief Check if task has been cancelled
     * @return true if cancel() was called
     */
    bool isCancelled() const {
        return m_cancelled.load(std::memory_order_acquire);
    }

    /**
     * @brief Get task ID
     * @return Unique task identifier
     */
    int id() const {
        return m_id;
    }

    /**
     * @brief Get task priority
     * @return Task priority level
     */
    TaskPriority priority() const {
        return m_priority;
    }

    /**
     * @brief Get task name (for debugging)
     * @return Human-readable task name
     */
    virtual QString name() const {
        return QString("Task #%1").arg(m_id);
    }

   signals:
    /**
     * @brief Emitted when task starts execution
     * @param taskId Task ID
     */
    void started(int taskId);

    /**
     * @brief Emitted to report task progress
     * @param taskId Task ID
     * @param percent Progress percentage (0-100)
     */
    void progress(int taskId, int percent);

    /**
     * @brief Emitted when task completes successfully
     * @param taskId Task ID
     * @param result Task result (type depends on task)
     */
    void completed(int taskId, QVariant result);

    /**
     * @brief Emitted when task fails with error
     * @param taskId Task ID
     * @param error Error message
     */
    void failed(int taskId, QString error);

    /**
     * @brief Emitted when task is cancelled
     * @param taskId Task ID
     */
    void cancelled(int taskId);

   protected:
    /**
     * @brief Execute task logic (implement in subclasses)
     *
     * This method runs on a worker thread. Should:
     * - Check isCancelled() periodically
     * - Emit progress() for long-running tasks
     * - Throw exceptions on error (caught by run())
     * - Return result via setResult()
     */
    virtual void execute() = 0;

    /**
     * @brief Update task progress
     * @param percent Progress percentage (0-100)
     */
    void setProgress(int percent);

    /**
     * @brief Set task result
     * @param result Task result value
     */
    void setResult(const QVariant& result);

   private:
    int m_id;
    TaskPriority m_priority;
    std::atomic<bool> m_cancelled;
    QVariant m_result;
};

/**
 * @brief Simple function-based task for lambda/function execution
 */
class FunctionTask : public Task {
    Q_OBJECT

   public:
    /**
     * @brief Construct a function task
     * @param id Task ID
     * @param function Function to execute
     * @param priority Task priority
     * @param parent Parent QObject
     */
    explicit FunctionTask(int id, std::function<void()> function,
                          TaskPriority priority = TaskPriority::NORMAL, QObject* parent = nullptr);

    QString name() const override {
        return QString("FunctionTask #%1").arg(id());
    }

   protected:
    void execute() override;

   private:
    std::function<void()> m_function;
};
