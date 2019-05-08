#pragma once

#include "array.h"
#include "bits.h"
#include "memorypolicy.h"
#include "refcounted.h"

#include <cassert>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace foundation
{
namespace detail
{

template <typename  Data,
          typename  Key,
          typename  MemoryPolicy,
          typename  Hash,
          typename  Equals,
          count_t   B>
struct hamt_node : public refcounted
{
    using data_t = Data;
    using key_t = Key;
    using memory_t = MemoryPolicy;
    using hash_fn = Hash;
    using equals_fn = Equals;

    using erase_res_t = std::variant<std::monostate, hamt_node*, data_t>;

    struct inner_t
    {
        bitmap_t    datamap;
        bitmap_t    nodemap;
        data_t*     data;
        hamt_node** children;
    };

    struct collision_t
    {
        std::size_t size;
        data_t*     data;
    };

    using impl_t = std::variant<inner_t, collision_t>;

    struct inner_tag {};
    struct collision_tag {};

    hamt_node(inner_tag)
      : m_impl(inner_t {0, 0, nullptr, nullptr})
    {}

    hamt_node(collision_tag)
      : m_impl(collision_t {0, nullptr})
    {}

    hamt_node(hamt_node&& other)
      : m_impl(std::move(other.m_impl))
    {}

    ~hamt_node()
    {
        if (is_inner())
        {
            auto n_first = children();
            auto n_size  = children_size();

            dec_children(n_first, n_first + n_size);
            memory_t::deallocate(n_first, n_size);

            auto d_first = data();
            auto d_size  = data_size();
            auto d_last  = d_first + d_size;

            for (; d_first < d_last; ++d_first)
            {
                memory_t::destroy(d_first);
            }

            memory_t::deallocate(data(), d_size);
        }
        else
        {
            auto c_first = collision_data();
            auto c_size  = collision_size();
            auto c_last  = c_first + c_size;

            for (; c_first < c_last; ++c_first)
            {
                memory_t::destroy(c_first);
            }

            memory_t::deallocate(collision_data(), c_size);
        }
    }

    auto& inner()
    {
        return std::get<inner_t>(m_impl);
    }

    const auto& inner() const
    {
        return std::get<inner_t>(m_impl);
    }

    bool is_inner() const
    {
        return std::get_if<inner_t>(&m_impl) != nullptr;
    }

    auto& collision()
    {
        return std::get<collision_t>(m_impl);
    }

    const auto& collision() const
    {
        return std::get<collision_t>(m_impl);
    }

    auto datamap() const
    {
        return inner().datamap;
    }

    auto nodemap() const
    {
        return inner().nodemap;
    }

    auto data() const
    {
        return inner().data;
    }

    auto children() const
    {
        return inner().children;
    }

    auto collision_data() const
    {
        return collision().data;
    }

    auto data_size() const
    {
        return popcount(datamap());
    }

    auto children_size() const
    {
        return popcount(nodemap());
    }

    auto collision_size() const
    {
        return collision().size;
    }

    static void inc_children(hamt_node** first, hamt_node** last)
    {
        for (; first < last; ++first)
        {
            (*first)->inc();
        }
    }

    static void dec_children(hamt_node** first, hamt_node** last)
    {
        for (; first < last; ++first)
        {
            if ((*first)->dec())
            {
                memory_t::destroy(*first);
                memory_t::deallocate(*first, 1);
            }
        }
    }

    static auto make_inner_n()
    {
        hamt_node* dest = memory_t::template allocate<hamt_node>(1);
        memory_t::construct(dest, inner_tag {});

        return dest;
    }

    static auto make_collision_n()
    {
        hamt_node* dest = memory_t::template allocate<hamt_node>(1);
        memory_t::construct(dest, collision_tag {});

        return dest;
    }

    static hamt_node* make_collision_n(data_t&& a, data_t&& b)
    {
        hamt_node* dest = make_collision_n();

        dest->collision().size = 2;
        dest->collision().data = make_array<memory_t, data_t>(2);

        memory_t::construct(dest->collision().data, std::move(a));
        memory_t::construct(dest->collision().data + 1, std::move(b));

        return dest;
    }
    
    hamt_node* shallow_copy() const
    {
        if (is_inner())
        {
            hamt_node* dest = make_inner_n();

            dest->inner().datamap = datamap();
            if (datamap())
            {
                dest->inner().data = make_array<memory_t>(data(), data_size());
            }

            dest->inner().nodemap = nodemap();
            if (nodemap())
            {
                dest->inner().children = make_array<memory_t>(
                    children(),
                    children_size());
            }

            inc_children(children(), children() + children_size());

            return dest;
        }
        else
        {
            hamt_node* dest = make_collision_n();

            dest->collision().size = collision_size();

            if (collision_size())
            {
                dest->collision().data = make_array<memory_t>(
                    collision_data(),
                    collision_size());
            }

            return dest;
        }
    }

    hamt_node* replace_value(data_t&& value, count_t compact_idx) const
    {
        hamt_node* dest = make_inner_n();

        dest->inner().datamap = datamap();
        dest->inner().data = make_array_replace<memory_t>(
            data(),
            data_size(),
            compact_idx,
            std::move(value));

        dest->inner().nodemap  = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    hamt_node* insert_value(data_t&& value, bitmap_t bit) const
    {
        hamt_node* dest = make_inner_n();

        auto compact_idx = popcount(datamap() & (bit - 1));

        dest->inner().datamap = datamap() | bit;
        dest->inner().data = make_array_insert<memory_t>(
            data(),
            data_size(),
            compact_idx,
            std::move(value));

        dest->inner().nodemap = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    hamt_node* erase_value(count_t d_compact_idx, bitmap_t bit) const
    {
        hamt_node* dest = make_inner_n();

        dest->inner().datamap = datamap() & ~bit;
        dest->inner().data = make_array_erase<memory_t>(
            data(),
            data_size(),
            d_compact_idx);

        dest->inner().nodemap = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    hamt_node* replace_child(hamt_node* new_n, count_t compact_idx) const
    {
        hamt_node* dest = make_inner_n();

        auto first = children();
        auto size  = children_size();

        dest->inner().datamap = datamap();
        dest->inner().data = data();
        dest->inner().nodemap = nodemap();
        dest->inner().children = make_array_replace<memory_t>(
            first,
            size,
            compact_idx,
            new_n);

        inc_children(first, first + compact_idx);
        inc_children(first + compact_idx + 1, first + size);

        return dest;
    }

    hamt_node* insert_child_erase_value(
        hamt_node*  child,
        count_t     d_compact_idx,
        bitmap_t    bit) const
    {
        hamt_node* dest = make_inner_n();

        auto n_compact_idx = popcount(nodemap() & (bit - 1));
        auto first = children();
        auto size = children_size();

        dest->inner().datamap = datamap() & ~bit;
        dest->inner().data = make_array_erase<memory_t>(
            data(),
            data_size(),
            d_compact_idx);

        dest->inner().nodemap = nodemap() | bit;
        dest->inner().children = make_array_insert<memory_t>(
            first,
            size,
            n_compact_idx,
            child);

        inc_children(first, first + size);

        return dest;
    }

    hamt_node* insert_value_erase_child(
        data_t&&    value,
        count_t     n_compact_idx,
        bitmap_t    bit) const
    {
        hamt_node* dest = make_inner_n();

        auto d_compact_idx = popcount(datamap() & (bit - 1));
        auto first = children();
        auto size = children_size();

        dest->inner().nodemap = nodemap() & ~bit;
        dest->inner().children = make_array_erase<memory_t>(
            first,
            size,
            n_compact_idx);

        dest->inner().datamap = datamap() | bit;
        dest->inner().data = make_array_insert<memory_t>(
            data(),
            data_size(),
            d_compact_idx,
            std::move(value));

        inc_children(first, first + n_compact_idx);
        inc_children(first + n_compact_idx + 1, first + size);

        return dest;
    }

    hamt_node* replace_collision(data_t&& value, count_t idx) const
    {
        hamt_node* dest = make_collision_n();

        dest->collision().size = collision_size();
        dest->collision().data = make_array_replace<memory_t>(
            collision_data(),
            collision_size(),
            idx,
            std::move(value));

        return dest;
    }

    hamt_node* insert_collision(data_t&& value) const
    {
        hamt_node* dest = make_collision_n();

        dest->collision().size = collision_size() + 1;
        dest->collision().data = make_array_insert<memory_t>(
            collision_data(),
            collision_size(),
            collision_size(),
            std::move(value));

        return dest;
    }

    hamt_node* erase_collision(count_t idx) const
    {
        assert(collision_size() > 2);

        hamt_node* dest = make_collision_n();

        dest->collision().size = collision_size() - 1;
        dest->collision().data = make_array_erase<memory_t>(
            collision_data(),
            collision_size(),
            idx);

        return dest;
    }

    bool operator==(const hamt_node& other) const
    {
        if (is_inner())
        {
            if (!other.is_inner())
            {
                return false;
            }

            if ((datamap() != other.datamap()) || (nodemap() != other.nodemap()))
            {
                return false;
            }

            auto this_data = data();
            auto last_data = this_data + data_size();
            auto other_data = other.data();

            for (; this_data != last_data; ++this_data, ++other_data)
            {
                if (*this_data != *other_data)
                {
                    return false;
                }
            }

            auto this_child = children();
            auto last_child = this_child + children_size();
            auto other_child = other.children();

            for (; this_child != last_child; ++this_child, ++other_child)
            {
                if (*this_child != *other_child)
                {
                    return false;
                }
            }

            return true;
        }

        //
        //  This is collision node.
        //

        if (other.is_inner())
        {
            return false;
        }

        if (collision_size() != other.children_size())
        {
            return false;
        }

        auto this_collision = collision_data();
        auto last_collision = this_collision + collision_size();
        auto other_collision = other.collision_data();

        for (;this_collision != last_collision; ++this_collision, ++other_collision)
        {
            if (*this_collision != *other_collision)
            {
                return false;
            }
        }

        return true; 
    }

    hamt_node* merge(
        data_t&&    a,
        hash_t      a_hash,
        data_t&&    b,
        hash_t      b_hash,
        shift_t     shift) const
    {
        auto a_sparse_idx = (a_hash >> shift) & (mask<B>);
        auto b_sparse_idx = (b_hash >> shift) & (mask<B>);

        if (a_sparse_idx != b_sparse_idx)
        {
            hamt_node* dest = make_inner_n();
            auto a_bit = bitmap_t {1u} << a_sparse_idx;
            auto b_bit = bitmap_t {1u} << b_sparse_idx;

            dest->inner().datamap = a_bit | b_bit;
            dest->inner().data = make_array<memory_t, data_t>(2);

            memory_t::construct(
                dest->inner().data + (a_bit > b_bit),
                std::move(a));

            memory_t::construct(
                dest->inner().data + (b_bit > a_bit),
                std::move(b));

            return dest;
        }

        hamt_node* dest = make_inner_n();

        auto bit = bitmap_t {1u} << a_sparse_idx;
        dest->inner().nodemap = bit;
        dest->inner().children = make_array<memory_t, hamt_node*>(1);

        if (shift + B >= max_shift<B>)
        {
            auto collision_n = make_collision_n(
                std::move(a),
                std::move(b));

            memory_t::construct(dest->inner().children, collision_n);

            return dest;
        }

        auto inner_n = dest->merge(
            std::move(a),
            a_hash,
            std::move(b),
            b_hash,
            shift + B);

        memory_t::construct(dest->inner().children, inner_n);
        return dest;
    }

    hamt_node* set(
        data_t&&        value,
        hash_t          hash,
        shift_t         shift) const
    {
        if (shift >= max_shift<B>)
        {
            // We reached maximum tree depth. This is collision node.
            for (std::size_t idx = 0; idx < collision_size(); ++idx)
            {
                auto data_ptr = collision_data() + idx;
                if (equals_fn {}(*data_ptr, value))
                {
                    return replace_collision(std::move(value), idx);
                }
            }

            return insert_collision(std::move(value));
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit = bitmap_t {1u} << sparse_idx;

        if (bit & datamap())
        {
            auto compact_idx = popcount(datamap() & (bit - 1));
            auto& prev_value = *(data() + compact_idx);

            if (equals_fn {}(prev_value, value))
            {
                return replace_value(std::move(value), compact_idx);
            }

            // Don't calculate hash if it's not needed.
            if (shift + B >= max_shift<B>)
            {
                auto child = make_collision_n(
                    std::move(prev_value),
                    std::move(value));

                return insert_child_erase_value(child, compact_idx, bit);
            }

            auto old_hash = hash_fn {}(prev_value);

            auto child = merge(
                std::move(prev_value),
                old_hash,
                std::move(value),
                hash,
                shift + B);

            return insert_child_erase_value(child, compact_idx, bit);
        }
        else if (bit & nodemap())
        {
            auto compact_idx = popcount(nodemap() & (bit - 1));
            auto child = *(children() + compact_idx);

            auto res = child->set(std::move(value), hash, shift + B);

            if (res)
            {
                return replace_child(res, compact_idx);
            }

            return {};
        }

        return insert_value(std::move(value), bit);
    }

    data_t* get(
        const key_t&   key,
        hash_t         hash,
        shift_t        shift) const
    {
        if (shift >= max_shift<B>)
        {
            auto value_ptr = collision_data();
            auto last = value_ptr + collision_size();

            for (; value_ptr < last; ++value_ptr)
            {
                if (equals_fn {}(*value_ptr, key))
                {
                    return value_ptr;
                }
            }

            return nullptr;
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit = bitmap_t {1u} << sparse_idx;

        if (bit & datamap())
        {
            auto compact_idx = popcount(datamap() & (bit - 1));
            auto value_ptr = data() + compact_idx;

            if (equals_fn {}(*value_ptr, key))
            {
                return value_ptr;
            }
        }
        else if (bit & nodemap())
        {
            auto compact_idx = popcount(nodemap() & (bit - 1));
            auto child = *(children() + compact_idx);

            return child->get(key, hash, shift + B);
        }

        return nullptr;
    }

    erase_res_t erase(
        const key_t&    key,
        hash_t          hash,
        shift_t         shift) const
    {
        if (shift >= max_shift<B>)
        {
            for (std::size_t idx = 0; idx < collision_size(); ++idx)
            {
                auto value = *(collision_data() + idx);
                if (equals_fn {}(value, key))
                {
                    if (collision_size() > 2)
                    {
                        return erase_collision(idx);
                    }

                    return *(collision_data() + (idx == 0));
                }
            }

            return {};
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit = bitmap_t {1u} << sparse_idx;

        if (bit & datamap())
        {
            auto compact_idx = popcount(datamap() & (bit - 1));
            auto value = *(data() + compact_idx);

            if (equals_fn {}(value, key))
            {
                if ((shift == 0) || (children_size() > 0)
                    || (data_size() > 2))
                {
                    return erase_value(compact_idx, bit);
                }

                // We know now that this is not the root node, the node has no
                // children and hence there is exactly 2 values (1 value no
                // children case only allowed in root node).

                return *(data() + (compact_idx == 0));
            }

            return {};
        }
        else if (bit & nodemap())
        {
            auto compact_idx = popcount(nodemap() & (bit - 1));
            auto child = *(children() + compact_idx);

            auto res = child->erase(key, hash, shift + B);

            if (auto n = std::get_if<hamt_node*>(&res))
            {
                return replace_child(*n, compact_idx);
            }
            else if (auto v = std::get_if<data_t>(&res))
            {
                if ((shift == 0) || (children_size() > 1)
                    || (data_size() > 0))
                {
                    return insert_value_erase_child(
                        std::move(*v),
                        compact_idx,
                        bit);
                }

                return std::move(res);
            }

            return {};
        }

        return {};
    }

    impl_t m_impl;
};

} // namespace detail
} // namespace foundation
