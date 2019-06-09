#pragma once

#include "bits.h"
#include "hamtnode.h"

#include <algorithm>
#include <stack>
#include <utility>
#include <tuple>

namespace foundation
{
namespace immutable
{
namespace detail
{

template <
    typename  Data,
    typename  Key,
    typename  MemoryPolicy,
    typename  Hash,
    typename  Equals,
    CountType B>
class Iterator
{
    using Node = HamtNode<Data,
                          Key,
                          MemoryPolicy,
                          Hash,
                          Equals,
                          B>;

  public:
    Iterator() = default;
    explicit Iterator(Node* const* const root)
    {
        if ((*root)->children_size() != 0 || (*root)->data_size() != 0)
        {
            m_way_to_root[++m_current_depth] = root;
            descent_and_define_data();
        }
    }

    bool operator==(const Iterator& other) const noexcept
    {
        return
            (m_current_depth == -1 &&
            other.m_current_depth == -1) ||
            std::tie(m_current_depth, m_way_to_root[m_current_depth], m_this_data) ==
            std::tie(other.m_current_depth, other.m_way_to_root[m_current_depth], other.m_this_data);
    }

    bool operator!=(const Iterator& other) const noexcept
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
        Iterator result(*this);

        operator++();

        return result;
    }

    const Data& operator*() const
    {
        return *(m_this_data);
    }

  private:
    void next_node() noexcept
    {
        auto current_node = m_way_to_root[m_current_depth];

        if (!am_i_end())
        {
            if (m_this_data + 1 != m_last_data)
            {
                ++m_this_data;
            }
            else if (m_current_depth != 0)
            {
                while (!am_i_end())
                {
                    current_node = m_way_to_root[m_current_depth];
                    if (!((*current_node)->is_inner()) || !can_i_shift())
                    {
                        --m_current_depth;
                    }
                    else
                    {
                       ++m_way_to_root[m_current_depth];
                       descent_and_define_data();
                       break;
                    }

                    if (m_current_depth == 0 && (*m_way_to_root[m_current_depth])->data_size() == 0)
                    {
                        transform_to_end();
                    }
                }
            }
            else
            {
                transform_to_end();
            }

            m_way_to_root.pop();
        }
    }

    void descent_and_define_data() noexcept
    {
        auto current_node = m_way_to_root[m_current_depth];
        auto inner = (*current_node)->is_inner();
        while ((*current_node)->children_size())
        {
            current_node = (*current_node)->children();
            m_way_to_root[++m_current_depth] = current_node;
            inner = (*current_node)->is_inner();

            if (!inner)
            {
                break;
            }
        }

        define_data(inner);
    }

    void define_data(bool inner) noexcept {
        if (inner)
        {
            m_this_data = (*m_way_to_root[m_current_depth])->data();
            m_last_data = m_this_data + (*m_way_to_root[m_current_depth])->data_size();
        }
        else
        {
            m_this_data = (*m_way_to_root[m_current_depth])->collision_data();
            m_last_data = m_this_data + (*m_way_to_root[m_current_depth])->collision_size();
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

    std::array<Node* const*, (max_depth<B> + 1)> m_way_to_root;
    int                                          m_current_depth = -1;
    Data*                                        m_this_data;
    Data*                                        m_last_data;
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
          foundation::immutable::detail::CountType B>
struct iterator_traits<foundation::immutable::detail::Iterator<T,
                                                      Key,
                                                      MemoryPolicy,
                                                      Hash,
                                                      Equals,
                                                      B>>
{
    using ValueType = const T;
    using Pointer = const T*;
    using Reference	= const T&;
};

}
