#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace foundation
{
namespace immutable
{

template<typename T> struct full_decay : std::decay<T> {};
template<typename T> struct full_decay<T*> { using type = typename full_decay<T>::type*; };
template<typename T> struct full_decay<T * const> { using type = typename full_decay<T>::type*; };
template<typename T> using full_decay_t = typename full_decay<T>::type;


template<typename> struct is_int_base : std::false_type {};

template<> struct is_int_base<int8_t>   : std::true_type {};
template<> struct is_int_base<uint8_t>  : std::true_type {};
template<> struct is_int_base<int16_t>  : std::true_type {};
template<> struct is_int_base<uint16_t> : std::true_type {};
template<> struct is_int_base<int32_t>  : std::true_type {};
template<> struct is_int_base<uint32_t> : std::true_type {};
template<> struct is_int_base<int64_t>  : std::true_type {};
template<> struct is_int_base<uint64_t> : std::true_type {};

template<typename T> struct is_int : is_int_base<full_decay_t<T>> {};
template<typename T> inline constexpr bool is_int_v = is_int<T>::value;


template<typename> struct is_str_base : std::false_type {};

template<> struct is_str_base<std::string> : std::true_type {};
template<> struct is_str_base<char*>       : std::true_type {};

template<typename T> struct is_str : is_str_base<full_decay_t<T>> {};
template<typename T> inline constexpr bool is_str_v = is_str<T>::value;


template <typename T>
std::size_t get_hash() noexcept
{
    return typeid(T).hash_code();
}

std::array<std::size_t, 8> int_types
{
    get_hash<int8_t>(),
    get_hash<uint8_t>(),
    get_hash<int16_t>(),
    get_hash<uint16_t>(),
    get_hash<int32_t>(),
    get_hash<uint32_t>(),
    get_hash<int64_t>(),
    get_hash<uint64_t>()
};

std::array<std::size_t, 2> str_types
{
    get_hash<char*>(),
    get_hash<std::string>()
};

enum class Type
{
    str_types,
    int_types,
    random_types
};

class SharedAny
{
  public:
    template <typename T,
              std::enable_if_t<is_int_v<full_decay_t<T>>, int> = 0>
    SharedAny(T&& value)
      : m_data(new full_decay_t<T>(std::forward<T>(value)))
      , m_hash_type(get_hash<T>())
      , m_type(Type::int_types)
    {
        m_num = static_cast<int>(int_types.size()) - 1;
        for (; m_num != -1; --m_num)
        {
            if (m_hash_type == int_types[m_num])
            {
                break;
            }
        }
    }

    template <typename T,
              std::enable_if_t<is_str_v<full_decay_t<T>>, int> = 0>
    SharedAny(T&& value)
      : m_data(new std::string(std::forward<T>(value)))
      , m_hash_type(get_hash<T>())
      , m_type(Type::str_types)
    {}

    template <typename T,
              std::enable_if_t<!is_str_v<full_decay_t<T>>, int> = 0,
              std::enable_if_t<!is_int_v<full_decay_t<T>>, int> = 0>
    SharedAny(T&& value)
      : m_data(new full_decay_t<T>(std::forward<T>(value)))
      , m_hash_type(get_hash<T>())
      , m_type(Type::random_types)
    {}

    template <typename T>
    std::shared_ptr<T> pure_cast() const
    {
        if (!pure_is<T>())
        {
            throw std::bad_cast();
        }
        else
        {
            return std::reinterpret_pointer_cast<T>(m_data);
        }
    }

    template <typename T>
    bool pure_is() const noexcept
    {
        return get_hash<T>() == m_hash_type;
    }

    template <typename T,
              std::enable_if_t<is_int_v<full_decay_t<T>>, int> = 0>
    std::shared_ptr<T> cast() const
    {
        if (!is<T>())
        {
            throw std::bad_cast();
        }
        else
        {
            T val;

            switch (m_num)
            {
                case 0: val = (static_cast<T>(*(std::reinterpret_pointer_cast<int8_t>(m_data)))); break;
                case 1: val = (static_cast<T>(*(std::reinterpret_pointer_cast<uint8_t>(m_data)))); break;
                case 2: val = (static_cast<T>(*(std::reinterpret_pointer_cast<int16_t>(m_data)))); break;
                case 3: val = (static_cast<T>(*(std::reinterpret_pointer_cast<uint16_t>(m_data)))); break;
                case 4: val = (static_cast<T>(*(std::reinterpret_pointer_cast<int32_t>(m_data)))); break;
                case 5: val = (static_cast<T>(*(std::reinterpret_pointer_cast<uint32_t>(m_data)))); break;
                case 6: val = (static_cast<T>(*(std::reinterpret_pointer_cast<int64_t>(m_data)))); break;
                case 7: val = (static_cast<T>(*(std::reinterpret_pointer_cast<uint64_t>(m_data)))); break;
            }

            return std::make_shared<T>(val);
        }
    }

    template <typename T,
              std::enable_if_t<std::is_same_v<full_decay_t<T>, std::string>, int> = 0>
    std::shared_ptr<T> cast() const
    {
        if (!is<T>())
        {
            throw std::bad_cast();
        }
        else
        {
            std::string str = *std::reinterpret_pointer_cast<std::string>(m_data);
            return std::make_shared<T>(str);
        }
    }

    template <typename T,
              std::enable_if_t<is_str_v<full_decay_t<T>>, int> = 0,
              std::enable_if_t<!std::is_same_v<full_decay_t<T>, std::string>, int> = 0>
    std::shared_ptr<T> cast() const
    {
        if (!is<T>())
        {
            throw std::bad_cast();
        }
        else
        {
            std::string str = *std::reinterpret_pointer_cast<std::string>(m_data);
            return std::make_shared<T>(str.data());
        }
    }

    template <typename T,
              std::enable_if_t<!is_str_v<full_decay_t<T>>, int> = 0,
              std::enable_if_t<!is_int_v<full_decay_t<T>>, int> = 0>
    std::shared_ptr<T> cast() const
    {
        if (!is<T>())
        {
            throw std::bad_cast();
        }
        else
        {
            return std::reinterpret_pointer_cast<T>(m_data);
        }
    }

    template <typename T,
              std::enable_if_t<is_int_v<full_decay_t<T>>, int> = 0>
    bool is() const noexcept
    {
        return m_type == Type::int_types;
    }

    template <typename T,
              std::enable_if_t<is_str_v<full_decay_t<T>>, int> = 0>
    bool is() const noexcept
    {
        return m_type == Type::str_types;
    }

    template <typename T,
              std::enable_if_t<!is_str_v<full_decay_t<T>>, int> = 0,
              std::enable_if_t<!is_int_v<full_decay_t<T>>, int> = 0>
    bool is() const noexcept
    {
        return pure_is<T>();
    }

  private:
    std::shared_ptr<void>  m_data;
    const std::size_t      m_hash_type;
    int                    m_num;
    Type                   m_type;
};

} // namespace immutable
} // namespace foundation
