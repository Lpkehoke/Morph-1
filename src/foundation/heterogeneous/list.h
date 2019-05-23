#pragma once

#include "foundation/heterogeneous/box.h"

#include <cassert>
#include <utility>
#include <iterator>


namespace foundation
{
namespace heterogeneous
{

class list
{
  public:
    struct iterator
    {
        using value_type = box;
        using pointer = box*;
        using reference = box&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator();
        iterator(list* elem);

        reference operator*();
        pointer operator->();
        iterator& operator++();
        bool operator!=(const iterator& other);

      private:
        list* m_current_elem;
    };

    list();
    ~list();

    template <typename T>
    void push_back(T&& value)
    {
        push_back<T>(std::forward<T>(value), std::default_delete<std::decay_t<T>>{});
    }

    template <typename T, typename Deleter>
    void push_back(T&& value, Deleter&& deleter)
    {
        auto l = push_back_generic();
        l->m_held.store(
            std::forward<T>(value),
            std::forward<Deleter>(deleter));
    }

    template <typename T>
    void push_back(std::shared_ptr<T> value)
    {
        auto l = push_back_generic();
        l->m_held.store(std::move(value));
    }

    bool empty() const;
    iterator begin();
    iterator end();

  private:
    list* push_back_generic();

    list*   m_next;
    box     m_held;
};

} // namespace heterogeneous
} // namespace foundation