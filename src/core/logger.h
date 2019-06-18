#pragma once

#include "foundation/observable.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace core
{

enum class severity_t
{
    trace,
    debug,
    info,
    warning,
    error,
    fatal
};

struct log_record_t
{
    std::string                           message;
    severity_t                            severity;
    std::chrono::system_clock::time_point timestamp;
};

using state_t = std::vector<log_record_t>;

class logger : public foundation::observable< state_t >
{
  public:
    static std::shared_ptr<logger> create_logger() noexcept;
    ~logger() override;

    void log(severity_t severity, const std::string& message);


  private:
    logger();

    void post_record_to_queue(log_record_t&& lr);

    struct impl_t;
    impl_t* impl;
};

} // namespace core
