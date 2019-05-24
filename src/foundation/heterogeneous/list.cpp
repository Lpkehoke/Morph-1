#include "foundation/heterogeneous/list.h"

#include <utility>

namespace foundation
{
namespace heterogeneous
{

list::iterator::iterator()
    : m_current_elem(nullptr)
{}

list::iterator::iterator(list* elem)
    : m_current_elem(elem)
{}

list::iterator::reference list::iterator::operator*()
{
    return m_current_elem->m_held;
}

list::iterator::pointer list::iterator::operator->()
{
    return &m_current_elem->m_held;
}

list::iterator& list::iterator::operator++()
{
    m_current_elem = m_current_elem->m_next;
    return *this;
}

bool list::iterator::operator!=(const iterator& other)
{
    return m_current_elem != other.m_current_elem;
}

list::list()
    : m_next(nullptr)
{}

list::list(box boxed_value)
    : m_next(nullptr)
    , m_held(std::move(boxed_value))
{}

list::~list()
{
    delete m_next;
}

void list::push_back(box boxed_value)
{
    if (!m_held)
    {
        assert(!m_next);
        m_held = std::move(boxed_value);
    }

    list* l = this;
    while(l->m_next)
    {
        l = l->m_next;
    }

    l->m_next = new list(std::move(boxed_value));
}

bool list::empty() const
{
    return static_cast<bool>(m_held);
}

list::iterator list::begin()
{
    return iterator(this);
}

list::iterator list::end()
{
    return iterator();
}

} // namespace heterogeneous
} // namespace foundation