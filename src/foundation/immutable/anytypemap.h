#pragma once

#include "sharedany.h"

#include <utility>
#include <cstddef>
#include <memory>
#include <typeindex>

namespace foundation
{
namespace immutable
{

template <typename Key,
          typename Hash = std::hash<Key>,
          typename MemoryPolicy = detail::HeapMemoryPolicy>
class AnyTypeMap
{
  public:
    AnyTypeMap() = default;

    AnyTypeMap set(Key key, SharedAny value)
    {
        return AnyTypeMap(m_map.set(std::move(key), std::move(value)));
    }

    std::size_t size() const noexcept
    {
        return m_map.size();
    }

    const SharedAny* get(const Key& key) const
    {
        return m_map.get(key);
    }

    const SharedAny* operator[](const Key& key) const
    {
        return m_map.get(key);
    }

    AnyTypeMap erase(const Key& key) const
    {
        return AnyTypeMap(m_map.erase(key));
    }

    template<typename ValueType>
    std::shared_ptr<ValueType> getattr(const Key& key)
    {
        auto el = m_map.get(key);
        if (el && (*el).template is<ValueType>())
        {
            return ((*el).template cast<ValueType>());
        }

        return nullptr;
    }

  private:
    using InnerMap = Map<Key, SharedAny, Hash, MemoryPolicy>;
    AnyTypeMap(InnerMap&& m)
      : m_map(std::move(m))
    {}

    InnerMap m_map;
};

} // immutable
} // foundation
