#pragma once

#include <QObject>

#include "asynctask.h"

class AsyncActionTask : public QObject, public Task
{
    Q_OBJECT
public:
    typedef std::function<void()> Action;
    typedef std::function<void(TaskPtr)> ActionWithPrevTask;

    AsyncActionTask(Action);

    void start() override;

private slots:
    void finished();

private:
    Action action_;
};
