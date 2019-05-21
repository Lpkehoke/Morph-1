#include "logger.h"

#include "foundation/taskqueue.h"
#include "foundation/observable.h"
#include "foundation/observable.cpp"

#include <tbb/spin_mutex.h>

#include <chrono>
#include <mutex>
#include <string>
#include <utility>

using namespace foundation;

namespace core
{

struct Logger::Impl
{
    Impl()
      : m_task_queue(TaskQueue::Priority::Low)
    {}

    State           m_state;
    TaskQueue       m_task_queue;
    tbb::spin_mutex m_state_mutex;
};

Logger::Logger()
  : m_impl(new Impl())
{}

Logger::~Logger()
{
    delete m_impl;
}

std::shared_ptr<Logger> Logger::create() noexcept
{
    return std::shared_ptr<Logger>(new Logger());
}

void Logger::log(Severity severity, std::string message)
{
    LogRecord lr;

    lr.severity = severity;
    lr.message = message;
    lr.timestamp = std::chrono::system_clock::now();

    post_record_to_queue(std::move(lr));
}

void Logger::post_record_to_queue(LogRecord&& lr)
{
    m_impl->m_task_queue.post([=]()
    {
        {
            tbb::spin_mutex::scoped_lock lock(m_impl->m_state_mutex);
            m_impl->m_state.push_back(std::move(lr));
        }

        // TODO: not synchronized access to m_state.
        notify(m_impl->m_state);
    });
}

} // namespace core
