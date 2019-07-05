#pragma once

#include <memory>
#include <type_traits>
#include <typeindex>

namespace foundation
{
namespace immutable
{

class SharedAny
{
  public:
    template<typename ValueType>
    SharedAny(ValueType&& value)
      : m_data(new std::decay_t<ValueType>(std::forward<ValueType>(value)))
      , m_type(std::type_index(typeid(ValueType)).hash_code())
    {}

    template<typename ValueType>
    std::shared_ptr<ValueType> cast() const
    {
        if (std::type_index(typeid(ValueType)).hash_code() != m_type)
        {
            throw std::bad_cast();
        }
        else
        {
            return std::reinterpret_pointer_cast<ValueType>(m_data);
        }
    }

    template<typename ValueType>
    bool is() const noexcept
    {
        return typeid(ValueType).hash_code() == m_type;
    }

  private:
    std::shared_ptr<void>  m_data;
    const std::size_t      m_type;
};

} // namespace immutable
} // namespace foundation
