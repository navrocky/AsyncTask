#pragma once

#include <exception>
#include <memory>
#include <list>
#include <functional>
#include <string>

class CancellationToken
{
};
typedef std::shared_ptr<CancellationToken> CancellationTokenPtr;


class Task;
typedef std::shared_ptr<Task> TaskPtr;

template <typename ResultType>
class LambdaTask;

class Task : public std::enable_shared_from_this<Task>
{
public:
    enum State
    {
        NotStarted,
        Running,
        Finished,
        Error
    };

    Task();
    virtual ~Task();

    State state() const { return state_; }

    bool isFinished() const;
    bool hasError() const;

    virtual void start() = 0;

    TaskPtr run();

    TaskPtr continueWith(Task* task);
    TaskPtr continueWith(const TaskPtr &task);
    TaskPtr continueWith(const std::function<void()>& action);
    TaskPtr continueWith(const std::function<void(TaskPtr)>& action);

    template <typename ResultType>
    TaskPtr continueWith(const std::function<ResultType()>& action)
    {
        return continueWith(std::make_shared<LambdaTask<ResultType>>(action));
    }

    template <typename ResultType>
    TaskPtr continueWith(const std::function<ResultType(TaskPtr)>& action)
    {
        return continueWith(std::make_shared<LambdaTask<ResultType>>(action));
    }

    static TaskPtr continueWhenAll(const std::list<TaskPtr> tasks, TaskPtr task);

    static TaskPtr create(const std::function<void()>& action);

    template <typename ResultType>
    static TaskPtr create(const std::function<ResultType()>& action)
    {
        return std::make_shared<LambdaTask<ResultType>>(action);
    }

    TaskPtr prevTask() const { return prevTask_; }

    // debug
    static void enableInstanceTracking(bool);
    static int instanceCount();

protected:
    void finish();
    void error(const std::string&errorString);

private:
    void execContinuations();

    typedef std::list<TaskPtr> TaskContinuations;
    TaskContinuations continuations_;
    CancellationTokenPtr cancellationToken_;
    State state_;
    TaskPtr prevTask_;
    std::string errorString_;
};

template <typename ResultType>
class TaskWithResult : public Task
{
public:
    ResultType result() const { return result_; }

    static TaskWithResult<ResultType>* cast(TaskPtr task)
    {
        return static_cast<TaskWithResult<ResultType>*>(task.get());
    }

    void setResult(const ResultType& val)
    {
        result_ = val;
    }

private:
    ResultType result_;
};

template <>
class TaskWithResult<void> : public Task
{
};

namespace Details
{

template <typename ResultType>
void execAction(TaskWithResult<ResultType>* task, std::function<ResultType()> action)
{
    task->setResult(action());
}

template <>
inline void execAction<void>(TaskWithResult<void>*, std::function<void()> action)
{
    action();
}

template <typename ResultType>
void execActionWithPrevTask(TaskWithResult<ResultType>* task, const std::function<ResultType(TaskPtr)>& action, TaskPtr prevTask)
{
    task->setResult(action(prevTask));
}

template <>
inline void execActionWithPrevTask<void>(TaskWithResult<void>*, const std::function<void(TaskPtr)>& action, TaskPtr prevTask)
{
    action(prevTask);
}

}

template <typename ResultType>
class LambdaTask : public TaskWithResult<ResultType>
{
public:
    typedef TaskWithResult<ResultType> Base;
    typedef std::function<ResultType()> Action;
    typedef std::function<ResultType(TaskPtr)> ActionWithPrevTask;

    LambdaTask()
    {
        init();
    }

    LambdaTask(const Action& action)
        : action_(action)
    {
        init();
    }

    LambdaTask(const ActionWithPrevTask& action)
        : actionWithPrevTask_(action)
    {
        init();
    }

    void setAction(const Action& v) { action_ = v; }

    void setAutoFinishEnabled(bool val) { autoFinishEnabled_ = val; }

    void start() override
    {
        try
        {
            if (action_)
            {
                Details::execAction(this, action_);
            }
            else if (actionWithPrevTask_)
            {
                Details::execActionWithPrevTask(this, actionWithPrevTask_, Task::prevTask());
            }
            if (autoFinishEnabled_)
                Task::finish();
        }
        catch (const std::exception& e)
        {
            Task::error(e.what());
        }
    }

    using Task::finish;

private:
    void init()
    {
        autoFinishEnabled_ = true;
    }

    Action action_;
    ActionWithPrevTask actionWithPrevTask_;
    bool autoFinishEnabled_;
};
