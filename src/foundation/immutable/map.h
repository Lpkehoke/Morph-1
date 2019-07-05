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

template <typename Key,
          typename Value,
          typename Hash = std::hash<Key>,
          typename MemoryPolicy = detail::HeapMemoryPolicy>
class Map
{
  public:
    static constexpr detail::CountType branches = 6u;
    using Data = std::pair<const Key, Value>;

    struct HashFn
    {
        detail::HashType operator()(const Data& value) const
        {
            return Hash {}(value.first);
        }
    };

    struct EqualsFn
    {
        bool operator()(const Data& lhs, const Data& rhs) noexcept
        {
            return lhs.first == rhs.first;
        }

        bool operator()(const Data& lhs, const Key& rhs) noexcept
        {
            return lhs.first == rhs;
        }
    };

    using Node = detail::HamtNode<Data,
                                  Key,
                                  MemoryPolicy,
                                  HashFn,
                                  EqualsFn,
                                  branches>;

    using Iterator = detail::Iterator<Data,
                                      Key,
                                      MemoryPolicy,
                                      HashFn,
                                      EqualsFn,
                                      branches>;

    Map()
        : m_size(0u)
    {
        m_root = MemoryPolicy::template allocate<Node>(1);
        MemoryPolicy::construct(m_root, typename Node::InnerTag {});
    }

    Map(const Map& other)
      : m_root(other.m_root)
      , m_size(other.m_size)
    {
        m_root->inc();
    }

    Map(Map&& other)
        : m_root(nullptr)
        , m_size(0u)
    {
        std::swap(m_root, other.m_root);
        std::swap(m_size, other.m_size);
    }

    Map& operator=(Map other)
    {
        std::swap(other.m_root, m_root);
        std::swap(other.m_size, m_size);
        return *this;
    }

    const Value* get(const Key& key) const
    {
        auto hash = Hash {}(key);
        auto res = m_root->get(key, hash, 0);

        if (res)
        {
            return &res->second;
        }

        return nullptr;
    }

    const Value* operator[](const Key& key) const
    {
        return get(key);
    }

    Map set(Key key, Value value) const
    {
        auto hash = Hash {}(key);

        auto res = m_root->set(
            Data {std::move(key), std::move(value)},
            hash,
            0);

        if (res)
        {
            return Map {res, m_size + 1};
        }

        return *this;
    }

    Map erase(const Key& key) const
    {
        auto hash = Hash {}(key);

        auto res = m_root->erase(key, hash, 0);

        if (auto n = std::get_if<Node*>(&res))
        {
            return Map {*n, m_size - 1};
        }
        else if (auto v = std::get_if<Data>(&res))
        {
            assert(false && "Wrong return value.");
        }

        return *this;
    }

    std::size_t size() const noexcept
    {
        return m_size;
    }

    Iterator begin() const noexcept
    {
        return Iterator(&m_root);
    }

    Iterator end() const noexcept
    {
        return Iterator();
    }

    bool operator==(const Map& other) const noexcept
    {
        if (m_root == other.m_root)
        {
            return true;
        }

        return *m_root == *other.m_root;
    }

    bool operator!=(const Map& other) const noexcept
    {
        return !(*this == other);
    }

    ~Map()
    {
        if (m_root && m_root->dec())
        {
            MemoryPolicy::destroy(m_root);
            MemoryPolicy::deallocate(m_root, 1);
        }
    }

  private:
    Map(Node* root, std::size_t size)
      : m_root(root)
      , m_size(size)
    {}

    Node*       m_root;
    std::size_t m_size;
};

} // namespace immutable
} // namespace foundation
