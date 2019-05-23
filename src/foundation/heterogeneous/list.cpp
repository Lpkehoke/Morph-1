#include "foundation/heterogeneous/list.h"

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

list::~list()
{
    delete m_next;
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

list* list::push_back_generic()
{
    if (!m_held)
    {
        assert(!m_next);
        return this;
    }

    list* l = this;
    while(l->m_next)
    {
        l = l->m_next;
    }

    auto res = new list();
    l->m_next = res;
    return res;
}

} // namespace heterogeneous
} // namespace foundation