#pragma once

#include <functional>

namespace foundation
{

class TaskQueue
{
  public:
    using Task = std::function<void()>;

    enum class Priority
    {
        Low,
        Normal,
        High
    };

    TaskQueue(Priority priority = Priority::Normal);
    ~TaskQueue();

    void post(Task task);

  private:
    struct Impl;
    Impl* m_impl;
};

} // namespace foundation
