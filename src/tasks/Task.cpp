#include "Task.h"

#include <QDebug>
#include <exception>

Task::Task(int id, TaskPriority priority, QObject* parent)
    : QObject(parent), m_id(id), m_priority(priority), m_cancelled(false) {
    // Enable automatic deletion after run() completes
    setAutoDelete(true);
}

void Task::run() {
    // Emit started signal
    Q_EMIT started(m_id);

    try {
        // Check if already cancelled before starting
        if (isCancelled()) {
            Q_EMIT cancelled(m_id);
            return;
        }

        // Execute task-specific logic
        execute();

        // Check if cancelled during execution
        if (isCancelled()) {
            Q_EMIT cancelled(m_id);
            return;
        }

        // Task completed successfully
        Q_EMIT completed(m_id, m_result);

    } catch (const std::exception& e) {
        // Task failed with exception
        qWarning() << "Task" << m_id << "failed:" << e.what();
        Q_EMIT failed(m_id, QString::fromUtf8(e.what()));

    } catch (...) {
        // Unknown exception
        qWarning() << "Task" << m_id << "failed with unknown exception";
        Q_EMIT failed(m_id, "Unknown error occurred");
    }
}

void Task::cancel() {
    m_cancelled.store(true, std::memory_order_release);
    qDebug() << "Task" << m_id << "cancellation requested";
}

void Task::setProgress(int percent) {
    // Clamp to valid range
    int clampedPercent = qBound(0, percent, 100);
    Q_EMIT progress(m_id, clampedPercent);
}

void Task::setResult(const QVariant& result) {
    m_result = result;
}

// FunctionTask implementation
FunctionTask::FunctionTask(int id, std::function<void()> function, TaskPriority priority,
                           QObject* parent)
    : Task(id, priority, parent), m_function(std::move(function)) {}

void FunctionTask::execute() {
    if (m_function) {
        m_function();
    }
}
