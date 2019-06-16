#include "taskqueue.h"

#include <tbb/concurrent_queue.h>
#include <tbb/spin_mutex.h>
#include <tbb/task.h>

#include <functional>
#include <utility>

namespace foundation
{
namespace
{

class TaskWrapper : public tbb::task
{
  public:
    using OnTaskFinished = std::function<void()>;

    TaskWrapper(TaskQueue::Task&& t, OnTaskFinished&& cb)
      : m_t(std::move(t))
      , m_cb(std::move(cb))
    {}

    virtual tbb::task* execute() override;

  private:
    TaskQueue::Task     m_t;
    OnTaskFinished      m_cb;
};

tbb::task* TaskWrapper::execute()
{
    if (m_t)
    {
        m_t();
    }

    if (m_cb)
    {
        m_cb();
    }

    return nullptr;
}

} // namespace

struct TaskQueue::Impl
{
    Impl(tbb::priority_t priority)
      : m_is_busy(false)
      , m_priority(priority)
    {}

    void enqueue_task(Task&& task);
    void on_task_finished();

    tbb::concurrent_queue<Task>     m_queue;
    tbb::task_group_context         m_ctx;
    tbb::spin_mutex                 m_mutex;
    bool                            m_is_busy;
    tbb::priority_t                 m_priority;
};

void TaskQueue::Impl::enqueue_task(Task&& task)
{
    TaskWrapper* w = new (tbb::task::allocate_root(m_ctx))
            TaskWrapper(std::move(task), [=]() { return on_task_finished(); });

    tbb::task::enqueue(*w, m_priority);
}

void TaskQueue::Impl::on_task_finished()
{
    tbb::spin_mutex::scoped_lock lock(m_mutex);

    if (m_ctx.is_group_execution_cancelled())
    {
        return;
    }

    Task task;
    if (m_queue.try_pop(task))
    {
        enqueue_task(std::move(task));
    }
    else
    {
        m_is_busy = false;
    }
}

TaskQueue::TaskQueue(Priority priority)
{
    tbb::priority_t tbb_priority(tbb::priority_normal);

    switch (priority)
    {
        case Priority::Low:
            tbb_priority = tbb::priority_low;
            break;

        case Priority::Normal:
            tbb_priority = tbb::priority_normal;
            break;

        case Priority::High:
            tbb_priority = tbb::priority_high;
            break;

        default:
            break;
    }

    m_impl = new Impl(tbb_priority);
}

TaskQueue::~TaskQueue()
{
    m_impl->m_ctx.cancel_group_execution();
    delete m_impl;
}

void TaskQueue::post(Task task)
{
    tbb::spin_mutex::scoped_lock lock(m_impl->m_mutex);

    if (!m_impl->m_is_busy)
    {
        m_impl->enqueue_task(std::move(task));
        m_impl->m_is_busy = true;

        return;
    }

    m_impl->m_queue.push(std::move(task));
}

} // namespace foundation
