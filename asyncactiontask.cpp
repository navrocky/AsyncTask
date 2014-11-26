#include "asyncactiontask.h"

#include <QtConcurrent>
#include <QFutureWatcher>

AsyncActionTask::AsyncActionTask(AsyncActionTask::Action action)
    : action_(action)

{

}

void AsyncActionTask::start()
{
    auto futureWatcher = new QFutureWatcher<void>(this);
    connect(futureWatcher, SIGNAL(finished()), SLOT(finished()));
    futureWatcher->setFuture(QtConcurrent::run(action_));
}

void AsyncActionTask::finished()
{
    finish();
}
