#pragma once

#include "bits.h"
#include "hamtnode.h"

#include <algorithm>
#include <stack>
#include <utility>
#include <tuple>

#include <iostream>

namespace foundation
{
namespace immutable
{
namespace detail
{

template <
    typename data_t,
    typename Key,
    typename MemoryPolicy,
    typename Hash,
    typename Equals,
    count_t  B>
class iterator_t
{
    using node_type = hamt_node<data_t,
                                Key,
                                MemoryPolicy,
                                Hash,
                                Equals,
                                B>;

  public:
    explicit iterator_t() = default;
    explicit iterator_t(node_type* const* root)
    {
        if ((*root)->children_size() != 0 || (*root)->data_size() != 0) {
            m_way_to_root[++m_current_depth] = root;
            discent();
        }
    }

    bool operator==(const iterator_t& other) const noexcept
    {
        return
            (m_current_depth == -1 &&
            other.m_current_depth == -1) ||
            std::tie(m_current_depth, m_way_to_root[m_current_depth], m_data_id) ==
            std::tie(other.m_current_depth, other.m_way_to_root[m_current_depth], other.m_data_id);
    }

    bool operator!=(const iterator_t& other) const noexcept
    {
        return !(*this == other);
    }

    auto operator++() noexcept
    {
        next_node();
        return *this;
    }

    auto operator++(int) noexcept
    {
        iterator_t result(*this);

        operator++();

        return result;
    }

    const data_t& operator*() const
    { // error on dereference end iterator
        return *((*m_way_to_root[m_current_depth])->data() + m_data_id);
    }

  private:
    void next_node() noexcept
    {
        if (!am_i_end())
        {
            auto current_node = m_way_to_root[m_current_depth];
            if ((*current_node)->data_size() > m_data_id + 1)
            {
                ++m_data_id;
            }
            else if (m_current_depth != 0)
            {
                while (!am_i_end())
                {
                    if (can_i_shift())
                    {
                       ++m_way_to_root[m_current_depth];
                       discent();
                       break;
                    }
                    else
                    {
                        --m_current_depth;

                        if (m_current_depth == 0 && (*m_way_to_root[m_current_depth])->data_size() == 0)
                        {
                            transform_to_end();
                        }
                    }
                }

                m_data_id = 0;
            }
            else
            {
                transform_to_end();
            }
        }
    }

    void discent() noexcept
    {
        auto current_node = m_way_to_root[m_current_depth];
        while ((*current_node)->children_size())
        {
            current_node = (*current_node)->children();
            m_way_to_root[++m_current_depth] = current_node;
        }
    }

    bool can_i_shift() const
    {
        auto current_node = m_way_to_root[m_current_depth];
        auto parent = m_way_to_root[m_current_depth-1];
        auto first = (*parent)->children();

        return ((*parent)->children_size() > current_node - first + 1);
    }

    void transform_to_end() noexcept
    {
        m_current_depth = -1;
    }

    bool am_i_end() const noexcept
    {
        return (m_current_depth == -1);
    }

    std::array<node_type* const*, max_depth<B>> m_way_to_root;
    int                                         m_current_depth = -1;
    std::size_t                                 m_data_id       =  0;
};

} // namespace detail
} // namespace immutable
} // namespace foundation
namespace std
{

template <typename T,
          typename Key,
          typename MemoryPolicy,
          typename Hash,
          typename Equals,
          foundation::immutable::detail::count_t B>
struct iterator_traits<foundation::immutable::detail::iterator_t<T,
                                                      Key,
                                                      MemoryPolicy,
                                                      Hash,
                                                      Equals,
                                                      B>>
{
    using value_type = const T;
    using pointer = const T*;
    using reference	= const T&;
};

}
