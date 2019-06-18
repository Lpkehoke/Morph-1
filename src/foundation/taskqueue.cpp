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

class task_wrapper : public tbb::task
{
  public:
    using on_task_end_t = std::function<void()>;

    task_wrapper(task_queue::task_t&& t, on_task_end_t&& cb)
      : m_t(std::move(t))
      , m_cb(std::move(cb))
    {}

    virtual tbb::task* execute() override;

  private:
    task_queue::task_t  m_t;
    on_task_end_t       m_cb;
};

tbb::task* task_wrapper::execute()
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

struct task_queue::impl_t
{
    impl_t(tbb::priority_t priority)
      : m_is_busy(false)
      , m_priority(priority)
    {}

    void enqueue_task(task_t&& task);
    void on_task_end_t();

    tbb::concurrent_queue<task_t> m_queue;
    tbb::task_group_context       m_ctx;
    tbb::spin_mutex               m_mutex;
    bool                          m_is_busy;
    tbb::priority_t               m_priority;
};

void task_queue::impl_t::enqueue_task(task_t&& task)
{
    task_wrapper* w = new (tbb::task::allocate_root(m_ctx))
            task_wrapper(std::move(task), [=]() { return on_task_end_t(); });

    tbb::task::enqueue(*w, m_priority);
}

void task_queue::impl_t::on_task_end_t()
{
    tbb::spin_mutex::scoped_lock lock(m_mutex);

    if (m_ctx.is_group_execution_cancelled())
    {
        return;
    }

    task_t task;
    if (m_queue.try_pop(task))
    {
        enqueue_task(std::move(task));
    }
    else
    {
        m_is_busy = false;
    }
}

task_queue::task_queue(priority_t priority)
{
    tbb::priority_t tbb_priority(tbb::priority_normal);

    switch (priority)
    {
        case priority_t::low:
            tbb_priority = tbb::priority_low;
            break;

        case priority_t::normal:
            tbb_priority = tbb::priority_normal;
            break;

        case priority_t::high:
            tbb_priority = tbb::priority_high;
            break;

        default:
            break;
    }

    m_impl = new impl_t(tbb_priority);
}

task_queue::~task_queue()
{
    m_impl->m_ctx.cancel_group_execution();
    delete m_impl;
}

void task_queue::post(task_t task)
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
