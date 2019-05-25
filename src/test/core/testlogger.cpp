#include "core/logger.h"

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;
using namespace core;
using namespace std;

TEST(logger_test, working_logger)
{
    vector<string> logs_str
    {
        "trace",
        "debug",
        "info",
        "warning",
        "error",
        "fatal"
    };

    vector<Severity> logs_severity
    {
        Severity::Trace,
        Severity::Debug,
        Severity::Info,
        Severity::Warning,
        Severity::Error,
        Severity::Fatal
    };

    auto a = Logger::create();

    auto un = a->subscribe(
        [&logs_str, &logs_severity](const State& state)
        {
            size_t size_state = static_cast<size_t>(state.size());
            for (size_t i = 0; i < size_state; ++i)
            {
                ASSERT_EQ(state[i].message, logs_str[i]);
                ASSERT_EQ(state[i].severity, logs_severity[i]);
            }
        });

    for(size_t i = 0; i < logs_str.size(); ++i)
    {
        a->log(logs_severity[i], logs_str[i]);
    }

    std::this_thread::sleep_for(0.1s);
}
