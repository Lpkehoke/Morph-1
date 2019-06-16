#pragma once

#include "foundation/observable.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace core
{

enum class Severity
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

struct LogRecord
{
    std::string                             message;
    Severity                                severity;
    std::chrono::system_clock::time_point   timestamp;
};

using State = std::vector<LogRecord>;

class Logger : public foundation::Observable<State>
{
  public:
    static std::shared_ptr<Logger> create() noexcept;
    ~Logger();

    void log(Severity severity, std::string message);

  private:
    Logger();

    void post_record_to_queue(LogRecord&& lr);

    struct Impl;
    Impl* m_impl;
};

} // namespace core
