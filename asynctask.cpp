#include "asynctask.h"

namespace
{
int taskInstances = 0;
bool instanceTrackingEnabled = false;
}

Task::Task()
    : state_(NotStarted)
{
    if (instanceTrackingEnabled)
        taskInstances++;
}

Task::~Task()
{
    if (instanceTrackingEnabled)
        taskInstances--;
}

bool Task::isFinished() const
{
    return state_ == Finished || state_ == Error;
}

bool Task::hasError() const
{
    return state_ == Error;
}

TaskPtr Task::run()
{
    start();
    return shared_from_this();
}

TaskPtr Task::continueWith(Task *task)
{
    return continueWith(TaskPtr(task));
}

TaskPtr Task::continueWith(const TaskPtr& task)
{
    continuations_.push_back(task);
    return task;
}

TaskPtr Task::continueWith(const std::function<void ()> &action)
{
    return continueWith(create(action));
}

TaskPtr Task::continueWith(const std::function<void (TaskPtr)> &action)
{
    return continueWith(std::make_shared<LambdaTask<void>>(action));
}

TaskPtr Task::continueWhenAll(const std::list<TaskPtr> tasks, TaskPtr task)
{
    auto counter = std::make_shared<int>();
    for (TaskPtr t : tasks)
    {
        t->continueWith([counter, task]()
        {
            if (counter.unique())
                task->start();
        });
    }
    return task;
}

TaskPtr Task::create(const std::function<void ()> &action)
{
    return std::make_shared<LambdaTask<void>>(action);
}

void Task::enableInstanceTracking(bool val)
{
    instanceTrackingEnabled = val;
}

int Task::instanceCount()
{
    return taskInstances;
}

void Task::finish()
{
    state_ = Finished;
    execContinuations();
}

void Task::error(const std::string& errorString)
{
    state_ = Error;
    errorString_ = errorString;
    execContinuations();
}

void Task::execContinuations()
{
    for (TaskPtr task : continuations_)
    {
        task->prevTask_ = shared_from_this();
        task->start();
    }
    continuations_.clear();
}
