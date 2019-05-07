#pragma once

#include "detail/memorypolicy.h"
#include "detail/hamtnode.h"

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>


namespace foundation
{


template <typename K,
          typename V,
          typename Hash = std::hash<K>,
          typename MemoryPolicy = detail::heap_memory_policy>
class immutable_map
{
  public:
    static constexpr detail::count_t branches = 6u;
    using key_t = K;
    using value_t = V;
    using data_t = std::pair<const key_t, value_t>;
    using memory_t = MemoryPolicy;

    struct hash_fn
    {
        detail::hash_t operator()(const data_t& value) const
        {
            return Hash {}(value.first);
        }
    };

    struct equals_fn
    {
        bool operator()(const data_t& lhs, const data_t& rhs)
        {
            return lhs.first == rhs.first;
        }

        bool operator()(const data_t& lhs, const key_t& rhs)
        {
            return lhs.first == rhs;
        }
    };

    using node_t = detail::hamt_node<data_t,
                                     key_t,
                                     memory_t,
                                     hash_fn,
                                     equals_fn,
                                     branches>;

    immutable_map()
        : m_size(0u)
    {
        m_root = memory_t::template allocate<node_t>(1);
        memory_t::construct(m_root, typename node_t::inner_tag {});
    }

    immutable_map(const immutable_map& other)
        : m_root(other.m_root)
        , m_size(other.m_size)
    {
        m_root->inc();
    }

    immutable_map(immutable_map&& other)
        : m_root(nullptr)
        , m_size(0u)
    {
        std::swap(m_root, other.m_root);
        std::swap(m_size, other.m_size);
    }

    immutable_map& operator=(immutable_map other)
    {
        std::swap(other.m_root, m_root);
        std::swap(other.m_size, m_size);
        return *this;
    }

    const value_t* get(const key_t& key) const
    {
        auto hash = Hash {}(key);
        auto res = m_root->get(key, hash, 0);

        if (res)
        {
            return &res->second;
        }

        return nullptr;
    }

    const value_t* operator[](const key_t& key) const
    {
        return get(key);
    }

    immutable_map set(key_t key, value_t value) const
    {
        auto hash = Hash {}(key);

        auto res = m_root->set(
            data_t {std::move(key), std::move(value)},
            hash,
            0);

        if (res)
        {
            return immutable_map {res, m_size + 1};
        }

        return *this;
    }

    immutable_map erase(const key_t& key) const
    {
        auto hash = Hash {}(key);

        auto res = m_root->erase(key, hash, 0);

        if (auto n = std::get_if<node_t*>(&res))
        {
            return immutable_map {*n, m_size - 1};
        }
        else if (auto v = std::get_if<data_t>(&res))
        {
            assert(false && "Wrong return value.");
        }

        return *this;
    }

    std::size_t size() const
    {
        return m_size;
    }

    bool operator==(const immutable_map& other) const
    {
        if (m_root == other.m_root)
        {
            return true;
        }

        return *m_root == *other.m_root;
    }

    ~immutable_map()
    {
        if (m_root && m_root->dec())
        {
            memory_t::destroy(m_root);
            memory_t::deallocate(m_root, 1);
        }
    }

  private:
    immutable_map(node_t* root, std::size_t size)
        : m_root(root)
        , m_size(size)
    {}

    node_t*     m_root;
    std::size_t m_size;
};

} // namespace foundation
