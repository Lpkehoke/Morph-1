#pragma once

#include <memory>
#include <type_traits>
#include <typeinfo>

namespace foundation
{
namespace immutable
{

class SharedAny final
{
  public:
    template<typename ValueType>
    SharedAny(ValueType&& value)
      : m_data(new std::decay_t<ValueType>(std::forward<ValueType>(value)))
      , m_type(typeid(value))
    {}

    template<typename ValueType>
    std::shared_ptr<ValueType> cast() const
    {
        if (typeid(ValueType) != m_type)
        {
            throw std::bad_cast();
        }
        else
        {
            return std::reinterpret_pointer_cast<ValueType>(m_data);
        }
    }

    const std::type_info& type_info() const noexcept
	{
		return m_type;
	}

  private:
    std::shared_ptr<void> m_data;
    const std::type_info& m_type;
};

} // namespace immutable
} // namespace foundation
