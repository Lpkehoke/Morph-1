#pragma once

#include <type_traits>

namespace py
{
namespace detail
{

template <typename T>
using clean_t = typename std::remove_pointer_t<
                        typename std::remove_cv_t<
                            typename std::remove_reference_t<T>>>;

template <typename T>
using enable_if_pointer_t = typename std::enable_if_t<std::is_pointer_v<T>, T>;

template <typename T>
using enable_if_reference_t = typename std::enable_if_t<std::is_reference_v<T>, T>;

template <typename T, typename Return = T>
using enable_if_copy_constructible_t = typename std::enable_if_t<
                                        std::is_copy_constructible_v<T>, Return>;

template <typename T, typename Return = T>
using enable_if_not_copy_constructible_t = typename std::enable_if_t<
                                            !std::is_copy_constructible_v<T>, Return>;


template <typename T>
auto value_to_pointer(clean_t<T>&& src)
{
    return &src;
}

template <typename T>
auto value_to_pointer(const clean_t<T>& src)
{
    return const_cast<clean_t<T>*>(&src);
}

template <typename T>
auto value_to_pointer(const clean_t<T>* src)
{
    return const_cast<clean_t<T>*>(src);
}

template <typename T>
enable_if_pointer_t<T> pointer_to_value(clean_t<T>* src)
{
    return src;
}

template <typename T>
enable_if_reference_t<T> pointer_to_value(clean_t<T>* src)
{
    return static_cast<T>(*src);
}

} // namespace detail
} // namespace py
