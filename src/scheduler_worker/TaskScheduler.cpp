#include "TaskScheduler.h"

#include <QDebug>
#include <QThread>
#include <algorithm>

#include "ImageProcessingTask.h"

TaskScheduler::TaskScheduler() : QObject(nullptr), m_nextTaskId(1), m_paused(false) {
    // Create thread pool
    m_threadPool = new QThreadPool(this);

    // Set default thread count to CPU core count
    int idealThreadCount = QThread::idealThreadCount();
    if (idealThreadCount > 0) {
        m_threadPool->setMaxThreadCount(idealThreadCount);
    } else {
        m_threadPool->setMaxThreadCount(4);  // Fallback to 4 threads
    }

    qDebug() << "TaskScheduler initialized with" << m_threadPool->maxThreadCount()
             << "worker threads";
}

TaskScheduler::~TaskScheduler() {
    qDebug() << "TaskScheduler shutting down...";

    // Cancel all tasks
    cancelAllTasks();

    // Wait for all tasks to complete (with timeout)
    if (!m_threadPool->waitForDone(5000)) {
        qWarning() << "Some tasks did not complete within timeout";
    }

    qDebug() << "TaskScheduler destroyed";
}

TaskScheduler& TaskScheduler::instance() {
    static TaskScheduler instance;
    return instance;
}

int TaskScheduler::submitTask(std::function<void()> function, TaskPriority priority) {
    if (m_paused) {
        qWarning() << "TaskScheduler is paused, task not submitted";
        return -1;
    }

    int taskId = nextTaskId();

    // Create function task (auto-deleted after execution)
    FunctionTask* task = new FunctionTask(taskId, std::move(function), priority);

    // Connect signals
    connectTaskSignals(task);

    // Register for tracking
    registerTask(taskId, task);

    // Submit to thread pool
    m_threadPool->start(task, static_cast<int>(priority));

    qDebug() << "Submitted FunctionTask" << taskId << "with priority" << static_cast<int>(priority);

    return taskId;
}

int TaskScheduler::submitImageProcessingTask(
    const cv::Mat& image, const std::vector<std::shared_ptr<ImageOperation>>& operations,
    TaskPriority priority) {
    if (m_paused) {
        qWarning() << "TaskScheduler is paused, task not submitted";
        return -1;
    }

    int taskId = nextTaskId();

    // Create image processing task (auto-deleted after execution)
    ImageProcessingTask* task = new ImageProcessingTask(taskId, image, operations, priority);

    // Connect signals
    connectTaskSignals(task);

    // Register for tracking
    registerTask(taskId, task);

    // Submit to thread pool
    m_threadPool->start(task, static_cast<int>(priority));

    qDebug() << "Submitted ImageProcessingTask" << taskId << "with" << operations.size()
             << "operations";

    return taskId;
}

void TaskScheduler::cancelTask(int taskId) {
    QMutexLocker locker(&m_taskMapMutex);

    auto it = m_activeTasks.find(taskId);
    if (it != m_activeTasks.end()) {
        it->second->cancel();
        qDebug() << "Cancelled task" << taskId;
    } else {
        qWarning() << "Task" << taskId << "not found for cancellation";
    }
}

void TaskScheduler::cancelAllTasks() {
    QMutexLocker locker(&m_taskMapMutex);

    qDebug() << "Cancelling" << m_activeTasks.size() << "active tasks";

    for (auto& pair : m_activeTasks) {
        pair.second->cancel();
    }
}

void TaskScheduler::pauseScheduler() {
    m_paused.store(true, std::memory_order_release);
    qDebug() << "TaskScheduler paused";
}

void TaskScheduler::resumeScheduler() {
    m_paused.store(false, std::memory_order_release);
    qDebug() << "TaskScheduler resumed";
}

void TaskScheduler::setMaxThreads(int count) {
    // Clamp to reasonable range
    int clampedCount = qBound(1, count, 16);

    m_threadPool->setMaxThreadCount(clampedCount);
    qDebug() << "Max thread count set to" << clampedCount;
}

int TaskScheduler::maxThreads() const {
    return m_threadPool->maxThreadCount();
}

int TaskScheduler::activeThreadCount() const {
    return m_threadPool->activeThreadCount();
}

bool TaskScheduler::waitForAll(int timeoutMs) {
    qDebug() << "Waiting for all tasks to complete (timeout:" << timeoutMs << "ms)";
    return m_threadPool->waitForDone(timeoutMs);
}

void TaskScheduler::connectTaskSignals(Task* task) {
    // Forward task signals to scheduler signals
    connect(task, &Task::started, this, &TaskScheduler::taskStarted);
    connect(task, &Task::progress, this, &TaskScheduler::taskProgress);
    connect(task, &Task::completed, this, &TaskScheduler::taskCompleted);
    connect(task, &Task::failed, this, &TaskScheduler::taskFailed);
    connect(task, &Task::cancelled, this, &TaskScheduler::taskCancelled);

    // Unregister task when it completes/fails/cancels
    connect(task, &Task::completed, this, [this](int taskId, QVariant) { unregisterTask(taskId); });
    connect(task, &Task::failed, this, [this](int taskId, QString) { unregisterTask(taskId); });
    connect(task, &Task::cancelled, this, [this](int taskId) { unregisterTask(taskId); });
}

int TaskScheduler::nextTaskId() {
    return m_nextTaskId.fetch_add(1, std::memory_order_relaxed);
}

void TaskScheduler::registerTask(int taskId, Task* task) {
    QMutexLocker locker(&m_taskMapMutex);
    m_activeTasks[taskId] = task;
}

void TaskScheduler::unregisterTask(int taskId) {
    QMutexLocker locker(&m_taskMapMutex);
    m_activeTasks.erase(taskId);
    qDebug() << "Task" << taskId << "unregistered. Active tasks:" << m_activeTasks.size();
}
