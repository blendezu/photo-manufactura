#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <opencv2/opencv.hpp>

#include "../Task.h"
#include "../TaskScheduler.h"
#include "brightness_adjust.h"
#include "contrast_adjust.h"

/**
 * @brief Unit tests for TaskScheduler and Task classes
 */
class TestScheduler : public QObject {
    Q_OBJECT

   private slots:
    void initTestCase();
    void cleanupTestCase();

    // Task tests
    void testTaskCreation();
    void testTaskCancellation();
    void testFunctionTask();

    // Scheduler tests
    void testSchedulerSingleton();
    void testSubmitSimpleTask();
    void testSubmitMultipleTasks();
    void testTaskCompletion();
    void testTaskCancellation_Scheduler();
    void testCancelAllTasks();
    void testPauseResume();
    void testThreadPoolConfiguration();

    // Image processing tests
    void testImageProcessingTask();
    void testImageProcessingWithMultipleOperations();
    void testImageProcessingCancellation();

   private:
    TaskScheduler* m_scheduler;
};

void TestScheduler::initTestCase() {
    qDebug() << "Starting TaskScheduler tests";
    m_scheduler = &TaskScheduler::instance();
}

void TestScheduler::cleanupTestCase() {
    qDebug() << "TaskScheduler tests completed";
}

void TestScheduler::testTaskCreation() {
    FunctionTask task(1, []() { qDebug() << "Test task"; }, TaskPriority::NORMAL);

    QCOMPARE(task.id(), 1);
    QCOMPARE(task.priority(), TaskPriority::NORMAL);
    QVERIFY(!task.isCancelled());
}

void TestScheduler::testTaskCancellation() {
    FunctionTask task(2, []() { QThread::msleep(100); }, TaskPriority::NORMAL);

    QVERIFY(!task.isCancelled());
    task.cancel();
    QVERIFY(task.isCancelled());
}

void TestScheduler::testFunctionTask() {
    bool executed = false;

    int taskId = m_scheduler->submitTask([&executed]() { executed = true; });

    QVERIFY(taskId > 0);

    // Wait for task to complete
    QTest::qWait(500);

    QVERIFY(executed);
}

void TestScheduler::testSchedulerSingleton() {
    TaskScheduler& instance1 = TaskScheduler::instance();
    TaskScheduler& instance2 = TaskScheduler::instance();

    QCOMPARE(&instance1, &instance2);
}

void TestScheduler::testSubmitSimpleTask() {
    QSignalSpy spyStarted(m_scheduler, &TaskScheduler::taskStarted);
    QSignalSpy spyCompleted(m_scheduler, &TaskScheduler::taskCompleted);

    int taskId = m_scheduler->submitTask([]() { QThread::msleep(10); });

    QVERIFY(taskId > 0);

    // Wait for signals
    QVERIFY(spyStarted.wait(1000));
    QVERIFY(spyCompleted.wait(1000));

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 1);

    // Verify task ID matches
    QCOMPARE(spyStarted.first().at(0).toInt(), taskId);
    QCOMPARE(spyCompleted.first().at(0).toInt(), taskId);
}

void TestScheduler::testSubmitMultipleTasks() {
    QSignalSpy spyCompleted(m_scheduler, &TaskScheduler::taskCompleted);

    std::atomic<int> counter{0};

    // Submit 5 tasks
    for (int i = 0; i < 5; ++i) {
        m_scheduler->submitTask([&counter]() {
            counter++;
            QThread::msleep(10);
        });
    }

    // Wait for all to complete
    QVERIFY(spyCompleted.wait(3000));
    QTest::qWait(500);  // Extra time for all tasks

    QCOMPARE(counter.load(), 5);
    QVERIFY(spyCompleted.count() >= 5);
}

void TestScheduler::testTaskCompletion() {
    QSignalSpy spyCompleted(m_scheduler, &TaskScheduler::taskCompleted);

    int taskId = m_scheduler->submitTask([]() { return; });

    QVERIFY(spyCompleted.wait(1000));

    QCOMPARE(spyCompleted.count(), 1);
    QCOMPARE(spyCompleted.first().at(0).toInt(), taskId);
}

void TestScheduler::testTaskCancellation_Scheduler() {
    QSignalSpy spyCancelled(m_scheduler, &TaskScheduler::taskCancelled);

    // Submit a long-running task
    int taskId = m_scheduler->submitTask([]() { QThread::msleep(2000); });

    // Cancel it immediately
    QTest::qWait(50);  // Give it time to start
    m_scheduler->cancelTask(taskId);

    // Should receive cancellation signal
    QVERIFY(spyCancelled.wait(1000));
    QCOMPARE(spyCancelled.first().at(0).toInt(), taskId);
}

void TestScheduler::testCancelAllTasks() {
    std::atomic<int> completed{0};

    // Submit multiple long tasks
    for (int i = 0; i < 5; ++i) {
        m_scheduler->submitTask([&completed]() {
            for (int j = 0; j < 100; ++j) {
                QThread::msleep(10);
            }
            completed++;
        });
    }

    // Cancel all
    QTest::qWait(50);
    m_scheduler->cancelAllTasks();

    QTest::qWait(500);

    // Not all should have completed
    QVERIFY(completed.load() < 5);
}

void TestScheduler::testPauseResume() {
    m_scheduler->pauseScheduler();
    QVERIFY(m_scheduler->isPaused());

    // Tasks should not be accepted when paused
    int taskId = m_scheduler->submitTask([]() {});
    QCOMPARE(taskId, -1);

    m_scheduler->resumeScheduler();
    QVERIFY(!m_scheduler->isPaused());

    // Tasks should be accepted now
    taskId = m_scheduler->submitTask([]() {});
    QVERIFY(taskId > 0);

    QTest::qWait(100);
}

void TestScheduler::testThreadPoolConfiguration() {
    int originalThreads = m_scheduler->maxThreads();

    m_scheduler->setMaxThreads(2);
    QCOMPARE(m_scheduler->maxThreads(), 2);

    m_scheduler->setMaxThreads(8);
    QCOMPARE(m_scheduler->maxThreads(), 8);

    // Restore original
    m_scheduler->setMaxThreads(originalThreads);
}

void TestScheduler::testImageProcessingTask() {
    QSignalSpy spyCompleted(m_scheduler, &TaskScheduler::taskCompleted);

    // Create a simple test image
    cv::Mat testImage(100, 100, CV_8UC3, cv::Scalar(128, 128, 128));

    // Create a simple operation
    std::vector<std::shared_ptr<ImageOperation>> operations;
    operations.push_back(std::make_shared<AdjustBrightness>(20));

    // Submit image processing task
    int taskId = m_scheduler->submitImageProcessingTask(testImage, operations);

    QVERIFY(taskId > 0);

    // Wait for completion
    QVERIFY(spyCompleted.wait(3000));

    // Verify result
    QVariant result = spyCompleted.first().at(1);
    QVERIFY(result.canConvert<cv::Mat>());

    cv::Mat resultMat = result.value<cv::Mat>();
    QVERIFY(!resultMat.empty());
    QCOMPARE(resultMat.cols, 100);
    QCOMPARE(resultMat.rows, 100);
}

void TestScheduler::testImageProcessingWithMultipleOperations() {
    QSignalSpy spyCompleted(m_scheduler, &TaskScheduler::taskCompleted);
    QSignalSpy spyProgress(m_scheduler, &TaskScheduler::taskProgress);

    // Create test image
    cv::Mat testImage(200, 200, CV_8UC3, cv::Scalar(100, 100, 100));

    // Create multiple operations
    std::vector<std::shared_ptr<ImageOperation>> operations;
    operations.push_back(std::make_shared<AdjustBrightness>(30));
    operations.push_back(std::make_shared<AdjustContrast>(20));
    operations.push_back(std::make_shared<AdjustBrightness>(-10));

    // Submit task
    int taskId = m_scheduler->submitImageProcessingTask(testImage, operations);

    // Wait for completion
    QVERIFY(spyCompleted.wait(5000));

    // Should have received progress updates
    QVERIFY(spyProgress.count() > 0);

    // Verify result
    cv::Mat resultMat = spyCompleted.first().at(1).value<cv::Mat>();
    QVERIFY(!resultMat.empty());
}

void TestScheduler::testImageProcessingCancellation() {
    QSignalSpy spyCancelled(m_scheduler, &TaskScheduler::taskCancelled);

    // Create large image for slower processing
    cv::Mat testImage(2000, 2000, CV_8UC3, cv::Scalar(128, 128, 128));

    // Create many operations
    std::vector<std::shared_ptr<ImageOperation>> operations;
    for (int i = 0; i < 20; ++i) {
        operations.push_back(std::make_shared<AdjustBrightness>(i - 10));
    }

    // Submit task
    int taskId = m_scheduler->submitImageProcessingTask(testImage, operations);

    // Cancel quickly
    QTest::qWait(50);
    m_scheduler->cancelTask(taskId);

    // Should receive cancellation
    QVERIFY(spyCancelled.wait(2000));
}

// Qt Test main
QTEST_MAIN(TestScheduler)
#include "test_scheduler.moc"
