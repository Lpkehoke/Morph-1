#pragma once

#include "foundation/heterogeneous/box.h"

#include <cassert>
#include <utility>


namespace foundation
{
namespace heterogeneous
{

class list
{
  public:
    list()
      : m_next(nullptr)
    {}

    ~list()
    {
        delete m_next;
    }

    template <typename T>
    void push_back(T&& value)
    {
        push_back<T>(std::forward<T>(value), std::default_delete<T>{});
    }

    template <typename T, typename Deleter>
    void push_back(T&& value, Deleter&& deleter)
    {
        if (!m_held)
        {
            assert(!m_next);
            m_held.store(
                std::forward<T>(value),
                std::forward<Deleter>(deleter));
            return;
        }

        m_next = new list();
        m_next->m_held.store(
            std::forward<T>(value),
            std::forward<Deleter>(deleter));
    }

    bool empty() const
    {
        return static_cast<bool>(m_held);
    }

  private:
    list*   m_next;
    box     m_held;
};

} // namespace heterogeneous
} // namespace foundation