#pragma once

#include "detail/memorypolicy.h"
#include "detail/iterator.h"
#include "detail/hamtnode.h"

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>


namespace foundation
{
namespace immutable
{

template <typename K,
          typename V,
          typename Hash = std::hash<K>,
          typename MemoryPolicy = detail::heap_memory_policy>
class map
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
        bool operator()(const data_t& lhs, const data_t& rhs) noexcept
        {
            return lhs.first == rhs.first;
        }

        bool operator()(const data_t& lhs, const key_t& rhs) noexcept
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

    using iterator_t  = detail::iterator_t<data_t,
                                           key_t,
                                           memory_t,
                                           hash_fn,
                                           equals_fn,
                                           branches>;

    map()
        : m_size(0u)
    {
        m_root = memory_t::template allocate<node_t>(1);
        memory_t::construct(m_root, typename node_t::inner_tag {});
    }

    map(const map& other)
        : m_root(other.m_root)
        , m_size(other.m_size)
    {
        m_root->inc();
    }

    map(map&& other)
        : m_root(nullptr)
        , m_size(0u)
    {
        std::swap(m_root, other.m_root);
        std::swap(m_size, other.m_size);
    }

    map& operator=(map other)
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

    map set(key_t key, value_t value) const
    {
        auto hash = Hash {}(key);

        auto res = m_root->set(
            data_t {std::move(key), std::move(value)},
            hash,
            0);

        if (res)
        {
            return map {res, m_size + 1};
        }

        return *this;
    }

    map erase(const key_t& key) const
    {
        auto hash = Hash {}(key);

        auto res = m_root->erase(key, hash, 0);

        if (auto n = std::get_if<node_t*>(&res))
        {
            return map {*n, m_size - 1};
        }
        else if (auto v = std::get_if<data_t>(&res))
        {
            assert(false && "Wrong return value.");
        }

        return *this;
    }

    std::size_t size() const noexcept
    {
        return m_size;
    }

    iterator_t begin() const noexcept
    {
        return iterator_t(&m_root);
    }

    iterator_t end() const noexcept
    {
        return iterator_t();
    }

    bool operator==(const map& other) const noexcept
    {
        if (m_root == other.m_root)
        {
            return true;
        }

        return *m_root == *other.m_root;
    }

    ~map()
    {
        if (m_root && m_root->dec())
        {
            memory_t::destroy(m_root);
            memory_t::deallocate(m_root, 1);
        }
    }

  private:
    map(node_t* root, std::size_t size)
        : m_root(root)
        , m_size(size)
    {}

    node_t*     m_root;
    std::size_t m_size;
};

} // namespace immutable
} // namespace foundation
