#include <QString>
#include <QtTest>
#include <QDebug>
#include <QTimer>
#include <QApplication>

#include "asynctask.h"
#include "asyncactiontask.h"

class SignalHandleTask : public QObject, public Task
{
    Q_OBJECT
public:
    void start() override {}
public slots:
    void finished()
    {
        finish();
    }
};

class TaskTest : public QObject
{
    Q_OBJECT
private slots:
    void simpleContinuation()
    {
        int count = 0;

        Task::enableInstanceTracking(true);
        {
            auto task = Task::create([&count]()
            {
                count += 1;
            });
            task->continueWith([&count]()
            {
                count += 2;
            });

            QVERIFY(Task::instanceCount() == 2);

            QVERIFY(count == 0);

            task->start();

            QVERIFY(count == 3);
        }

        QVERIFY(Task::instanceCount() == 0);
    }

    void continuationWithResult()
    {
        auto task = Task::create<QString>([]()
        {
            return QString("Hello");
        });

        auto resTask = task->continueWith<QString>([](TaskPtr prevTask)
        {
            auto prev = TaskWithResult<QString>::cast(prevTask);
            return prev->result() + " world!";
        });


        auto taskWithResult = TaskWithResult<QString>::cast(resTask);

        QVERIFY(taskWithResult->result() == "");
        task->start();
        QVERIFY(taskWithResult->result() == "Hello world!");
    }

    void asyncContinuationWithResult()
    {


        auto task = std::make_shared<AsyncActionTask>([]()
        {
            qDebug() << "Hello";
            QTest::qSleep(100);
        });

        task->continueWith([]()
        {
            qDebug() << "world!";
        });

        task->start();

        QTest::qSleep(1000);

    }

    void handleSignalTask()
    {
        QString res;

        auto task = std::make_shared<SignalHandleTask>();
        QTimer::singleShot(50, task.get(), SLOT(finished()));
        task->continueWith([&res]()
        {
            res += "Hello";
        })->continueWith([&res]()
        {
            res += " world!";
        });

        QVERIFY(res.isEmpty());

        QEventLoop loop;
        QTimer::singleShot(100, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY(res == "Hello world!");
    }

    TaskPtr createPauseTask(int msec)
    {
        auto task = std::make_shared<LambdaTask<void>>();
        task->setAutoFinishEnabled(false);
        task->setAction([task, msec]()
        {
            auto timeoutTask = std::make_shared<SignalHandleTask>();
            QTimer::singleShot(msec, timeoutTask.get(), SLOT(finished()));
            timeoutTask->continueWith([task]()
            {
                task->finish();
            });
        });
        return task;
    }

    void whenAll()
    {
        QElapsedTimer timer;
        timer.start();

        QEventLoop loop;

        int elapsed = 0;
        std::list<TaskPtr> list;
        list.push_back(createPauseTask(50)->run());
        list.push_back(createPauseTask(25)->run());
        list.push_back(createPauseTask(60)->run());
        Task::continueWhenAll(list, std::make_shared<LambdaTask<void>>([&timer, &loop, &elapsed]()
        {
            elapsed = timer.elapsed();
            loop.quit();
        }));

        loop.exec();

        QVERIFY(elapsed > 50 && elapsed < 70);
    }

    void compoundCommand()
    {
        QElapsedTimer timer;
        timer.start();

        QEventLoop loop;

        int elapsed = 0;
        std::list<TaskPtr> list;
        list.push_back(createPauseTask(50)->run());
        list.push_back(createPauseTask(25)->run());
        list.push_back(createPauseTask(60)->run());
        Task::continueWhenAll(list, std::make_shared<LambdaTask<void>>([&timer, &loop, &elapsed]()
        {
            elapsed = timer.elapsed();
            loop.quit();
        }));

        loop.exec();

        QVERIFY(elapsed > 50 && elapsed < 70);
    }

};

QTEST_MAIN(TaskTest)

#include "test.moc"
