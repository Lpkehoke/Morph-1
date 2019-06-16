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
namespace immutable
{
namespace detail
{

template <typename  Data,
          typename  Key,
          typename  MemoryPolicy,
          typename  Hash,
          typename  Equals,
          CountType B>
struct HamtNode : public Refcounted
{
    using EraseResult = std::variant<std::monostate, HamtNode*, Data>;

    struct Inner
    {
        BitmapType  datamap;
        BitmapType  nodemap;
        Data*       data;
        HamtNode**  children;
    };

    struct Collision
    {
        std::size_t size;
        Data*     data;
    };

    using Impl = std::variant<Inner, Collision>;

    struct InnerTag {};
    struct CollisionTag {};

    HamtNode(InnerTag)
      : m_impl(Inner {0, 0, nullptr, nullptr})
    {}

    HamtNode(CollisionTag)
      : m_impl(Collision {0, nullptr})
    {}

    HamtNode(HamtNode&& other)
      : m_impl(std::move(other.m_impl))
    {}

    ~HamtNode()
    {
        if (is_inner())
        {
            auto n_first = children();
            auto n_size = children_size();

            dec_children(n_first, n_first + n_size);
            MemoryPolicy::deallocate(n_first, n_size);

            auto d_first = data();
            auto d_size = data_size();
            auto d_last = d_first + d_size;

            for (; d_first < d_last; ++d_first)
            {
                MemoryPolicy::destroy(d_first);
            }

            MemoryPolicy::deallocate(data(), d_size);
        }
        else
        {
            auto c_first = collision_data();
            auto c_size = collision_size();
            auto c_last = c_first + c_size;

            for (; c_first < c_last; ++c_first)
            {
                MemoryPolicy::destroy(c_first);
            }

            MemoryPolicy::deallocate(collision_data(), c_size);
        }
    }

    auto& inner() noexcept
    {
        return std::get<Inner>(m_impl);
    }

    const auto& inner() const noexcept
    {
        return std::get<Inner>(m_impl);
    }

    bool is_inner() const noexcept
    {
        return std::get_if<Inner>(&m_impl) != nullptr;
    }

    auto& collision() noexcept
    {
        return std::get<Collision>(m_impl);
    }

    const auto& collision() const noexcept
    {
        return std::get<Collision>(m_impl);
    }

    auto datamap() const noexcept
    {
        return inner().datamap;
    }

    auto nodemap() const noexcept
    {
        return inner().nodemap;
    }

    auto data() const noexcept
    {
        return inner().data;
    }

    auto children() const noexcept
    {
        return inner().children;
    }

    auto collision_data() const noexcept
    {
        return collision().data;
    }

    auto data_size() const noexcept
    {
        return popcount(datamap());
    }

    auto children_size() const noexcept
    {
        return popcount(nodemap());
    }

    auto collision_size() const noexcept
    {
        return collision().size;
    }

    static void inc_children(HamtNode** first, HamtNode** last) noexcept
    {
        for (; first < last; ++first)
        {
            (*first)->inc();
        }
    }

    static void dec_children(HamtNode** first, HamtNode** last)
    {
        for (; first < last; ++first)
        {
            if ((*first)->dec())
            {
                MemoryPolicy::destroy(*first);
                MemoryPolicy::deallocate(*first, 1);
            }
        }
    }

    static auto make_inner_n()
    {
        HamtNode* dest = MemoryPolicy::template allocate<HamtNode>(1);
        MemoryPolicy::construct(dest, InnerTag {});

        return dest;
    }

    static auto make_collision_n()
    {
        HamtNode* dest = MemoryPolicy::template allocate<HamtNode>(1);
        MemoryPolicy::construct(dest, CollisionTag {});

        return dest;
    }

    static HamtNode* make_collision_n(Data&& a, Data&& b)
    {
        HamtNode* dest = make_collision_n();

        dest->collision().size = 2;
        dest->collision().data = make_array<MemoryPolicy, Data>(2);

        MemoryPolicy::construct(dest->collision().data, std::move(a));
        MemoryPolicy::construct(dest->collision().data + 1, std::move(b));

        return dest;
    }

    HamtNode* shallow_copy() const
    {
        if (is_inner())
        {
            HamtNode* dest = make_inner_n();

            dest->inner().datamap = datamap();
            if (datamap())
            {
                dest->inner().data = make_array<MemoryPolicy>(data(), data_size());
            }

            dest->inner().nodemap = nodemap();
            if (nodemap())
            {
                dest->inner().children = make_array<MemoryPolicy>(
                    children(),
                    children_size());
            }

            inc_children(children(), children() + children_size());

            return dest;
        }
        else
        {
            HamtNode* dest = make_collision_n();

            dest->collision().size = collision_size();

            if (collision_size())
            {
                dest->collision().data = make_array<MemoryPolicy>(
                    collision_data(),
                    collision_size());
            }

            return dest;
        }
    }

    HamtNode* replace_value(Data&& value, CountType compact_idx) const
    {
        HamtNode* dest = make_inner_n();

        dest->inner().datamap = datamap();
        dest->inner().data = make_array_replace<MemoryPolicy>(
            data(),
            data_size(),
            compact_idx,
            std::move(value));

        dest->inner().nodemap = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    HamtNode* insert_value(Data&& value, BitmapType bit) const
    {
        HamtNode* dest = make_inner_n();

        auto compact_idx = popcount(datamap() & (bit - 1));

        dest->inner().datamap = datamap() | bit;
        dest->inner().data = make_array_insert<MemoryPolicy>(
            data(),
            data_size(),
            compact_idx,
            std::move(value));

        dest->inner().nodemap = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    HamtNode* erase_value(CountType d_compact_idx, BitmapType bit) const
    {
        HamtNode* dest = make_inner_n();

        dest->inner().datamap = datamap() & ~bit;
        dest->inner().data = make_array_erase<MemoryPolicy>(
            data(),
            data_size(),
            d_compact_idx);

        dest->inner().nodemap = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    HamtNode* replace_child(HamtNode* new_n, CountType compact_idx) const
    {
        HamtNode* dest = make_inner_n();

        auto first = children();
        auto size = children_size();

        dest->inner().datamap = datamap();
        dest->inner().data = data();
        dest->inner().nodemap = nodemap();
        dest->inner().children = make_array_replace<MemoryPolicy>(
            first,
            size,
            compact_idx,
            new_n);

        inc_children(first, first + compact_idx);
        inc_children(first + compact_idx + 1, first + size);

        return dest;
    }

    HamtNode* insert_child_erase_value(
        HamtNode*   child,
        CountType   d_compact_idx,
        BitmapType  bit) const
    {
        HamtNode* dest = make_inner_n();

        auto n_compact_idx = popcount(nodemap() & (bit - 1));
        auto first = children();
        auto size = children_size();

        dest->inner().datamap = datamap() & ~bit;
        dest->inner().data = make_array_erase<MemoryPolicy>(
            data(),
            data_size(),
            d_compact_idx);

        dest->inner().nodemap = nodemap() | bit;
        dest->inner().children = make_array_insert<MemoryPolicy>(
            first,
            size,
            n_compact_idx,
            child);

        inc_children(first, first + size);

        return dest;
    }

    HamtNode* insert_value_erase_child(
        Data&&      value,
        CountType   n_compact_idx,
        BitmapType  bit) const
    {
        HamtNode* dest = make_inner_n();

        auto d_compact_idx = popcount(datamap() & (bit - 1));
        auto first = children();
        auto size = children_size();

        dest->inner().nodemap = nodemap() & ~bit;
        dest->inner().children = make_array_erase<MemoryPolicy>(
            first,
            size,
            n_compact_idx);

        dest->inner().datamap = datamap() | bit;
        dest->inner().data = make_array_insert<MemoryPolicy>(
            data(),
            data_size(),
            d_compact_idx,
            std::move(value));

        inc_children(first, first + n_compact_idx);
        inc_children(first + n_compact_idx + 1, first + size);

        return dest;
    }

    HamtNode* replace_collision(Data&& value, CountType idx) const
    {
        HamtNode* dest = make_collision_n();

        dest->collision().size = collision_size();
        dest->collision().data = make_array_replace<MemoryPolicy>(
            collision_data(),
            collision_size(),
            idx,
            std::move(value));

        return dest;
    }

    HamtNode* insert_collision(Data&& value) const
    {
        HamtNode* dest = make_collision_n();

        dest->collision().size = collision_size() + 1;
        dest->collision().data = make_array_insert<MemoryPolicy>(
            collision_data(),
            collision_size(),
            collision_size(),
            std::move(value));

        return dest;
    }

    HamtNode* erase_collision(CountType idx) const
    {
        assert(collision_size() > 2);

        HamtNode* dest = make_collision_n();

        dest->collision().size = collision_size() - 1;
        dest->collision().data = make_array_erase<MemoryPolicy>(
            collision_data(),
            collision_size(),
            idx);

        return dest;
    }

    bool operator==(const HamtNode& other) const  noexcept
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

    HamtNode* merge(
        Data&&      a,
        HashType    a_hash,
        Data&&      b,
        HashType    b_hash,
        ShiftType   shift) const
    {
        auto a_sparse_idx = (a_hash >> shift) & (mask<B>);
        auto b_sparse_idx = (b_hash >> shift) & (mask<B>);

        if (a_sparse_idx != b_sparse_idx)
        {
            HamtNode* dest = make_inner_n();
            auto a_bit = BitmapType {1u} << a_sparse_idx;
            auto b_bit = BitmapType {1u} << b_sparse_idx;

            dest->inner().datamap = a_bit | b_bit;
            dest->inner().data = make_array<MemoryPolicy, Data>(2);

            MemoryPolicy::construct(
                dest->inner().data + (a_bit > b_bit),
                std::move(a));

            MemoryPolicy::construct(
                dest->inner().data + (b_bit > a_bit),
                std::move(b));

            return dest;
        }

        HamtNode* dest = make_inner_n();

        auto bit = BitmapType {1u} << a_sparse_idx;
        dest->inner().nodemap = bit;
        dest->inner().children = make_array<MemoryPolicy, HamtNode*>(1);

        if (shift + B >= max_shift<B>)
        {
            auto collision_n = make_collision_n(
                std::move(a),
                std::move(b));

            MemoryPolicy::construct(dest->inner().children, collision_n);

            return dest;
        }

        auto inner_n = dest->merge(
            std::move(a),
            a_hash,
            std::move(b),
            b_hash,
            shift + B);

        MemoryPolicy::construct(dest->inner().children, inner_n);
        return dest;
    }

    HamtNode* set(
        Data&&      value,
        HashType    hash,
        ShiftType   shift) const
    {
        if (shift >= max_shift<B>)
        {
            // We reached maximum tree depth. This is collision node.
            for (std::size_t idx = 0; idx < collision_size(); ++idx)
            {
                auto data_ptr = collision_data() + idx;
                if (Equals {}(*data_ptr, value))
                {
                    return replace_collision(std::move(value), idx);
                }
            }

            return insert_collision(std::move(value));
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit = BitmapType {1u} << sparse_idx;

        if (bit & datamap())
        {
            auto compact_idx = popcount(datamap() & (bit - 1));
            auto& prev_value = *(data() + compact_idx);

            if (Equals {}(prev_value, value))
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

            auto old_hash = Hash {}(prev_value);

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

    Data* get(
        const Key&  key,
        HashType    hash,
        ShiftType   shift) const
    {
        if (shift >= max_shift<B>)
        {
            auto value_ptr = collision_data();
            auto last = value_ptr + collision_size();

            for (; value_ptr < last; ++value_ptr)
            {
                if (Equals {}(*value_ptr, key))
                {
                    return value_ptr;
                }
            }

            return nullptr;
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit = BitmapType {1u} << sparse_idx;

        if (bit & datamap())
        {
            auto compact_idx = popcount(datamap() & (bit - 1));
            auto value_ptr = data() + compact_idx;

            if (Equals {}(*value_ptr, key))
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

    EraseResult erase(
        const Key&  key,
        HashType    hash,
        ShiftType   shift) const
    {
        if (shift >= max_shift<B>)
        {
            for (std::size_t idx = 0; idx < collision_size(); ++idx)
            {
                auto value = *(collision_data() + idx);
                if (Equals {}(value, key))
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
        auto bit = BitmapType {1u} << sparse_idx;

        if (bit & datamap())
        {
            auto compact_idx = popcount(datamap() & (bit - 1));
            auto value = *(data() + compact_idx);

            if (Equals {}(value, key))
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

            if (auto n = std::get_if<HamtNode*>(&res))
            {
                return replace_child(*n, compact_idx);
            }
            else if (auto v = std::get_if<Data>(&res))
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

    Impl m_impl;
};

} // namespace detail
} // namespace immutable
} // namespace foundation
