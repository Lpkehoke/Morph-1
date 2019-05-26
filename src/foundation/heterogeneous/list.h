#pragma once

#include "foundation/heterogeneous/box.h"

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
    list(box boxed_value);
    ~list();

    void push_back(box boxed_value);

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