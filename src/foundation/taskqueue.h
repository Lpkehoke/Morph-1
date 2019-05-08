#pragma once

#include <functional>

namespace foundation
{

class task_queue
{
  public:
    using task_t = std::function<void()>;

    enum class priority_t
    {
        low,
        normal,
        high
    };

    task_queue(priority_t priority = priority_t::normal);
    ~task_queue();

    void post(task_t task);

  private:
    struct impl_t;
    impl_t* m_impl;
};

} // namespace foundation
