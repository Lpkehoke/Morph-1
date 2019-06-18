#include "logger.h"

#include "foundation/taskqueue.h"
#include "foundation/observable.h"

#include <tbb/spin_mutex.h>

#include <chrono>
#include <mutex>
#include <string>
#include <utility>

using namespace foundation;

namespace core
{

struct logger::impl_t
{
    impl_t()
      : m_task_queue(task_queue::priority_t::low)
    {}

    state_t         m_state;
    task_queue      m_task_queue;
    tbb::spin_mutex m_state_mutex;
};

logger::logger()
  : impl(new impl_t())
{}

logger::~logger()
{
    delete impl;
}

std::shared_ptr<logger> logger::create_logger() noexcept
{
    struct make_shared_enabler : public logger {};
    return std::make_shared<make_shared_enabler>();
}

void logger::log(severity_t severity, const std::string& message)
{
    log_record_t lr;

    lr.severity = severity;
    lr.message = message;
    lr.timestamp = std::chrono::system_clock::now();

    post_record_to_queue(std::move(lr));
}

void logger::post_record_to_queue(log_record_t&& lr)
{
    impl->m_task_queue.post([=]()
    {
        tbb::spin_mutex::scoped_lock lock(impl->m_state_mutex);
        impl->m_state.emplace_back(std::move(lr));

        lock.release();
        notify(impl->m_state);
    });
}

} // namespace core
