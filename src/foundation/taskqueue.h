#pragma once

#include <functional>

namespace foundation
{

class task_queue
{
  public:
    using task = std::function<void()>;

    enum class priority_t
    {
        Low,
        Normal,
        High
    };

    task_queue(priority_t priority = priority_t::Normal);
    ~task_queue();

    void post(task task);

  private:
    struct impl;
    impl* m_impl;
};

} // namespace foundation
