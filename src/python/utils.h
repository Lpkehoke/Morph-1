#pragma once

#include <type_traits>

namespace py
{
namespace detail
{

template <typename T>
using CleanType = typename std::remove_pointer_t<
                        typename std::remove_cv_t<
                            typename std::remove_reference_t<T>>>;

template <typename T>
using EnableIfPointer = typename std::enable_if_t<std::is_pointer_v<T>, T>;

template <typename T>
using EnableIfReference = typename std::enable_if_t<std::is_reference_v<T>, T>;

template <typename T, typename Return = T>
using EnableIfCopyConstructible = typename std::enable_if_t<
                                        std::is_copy_constructible_v<T>, Return>;

template <typename T, typename Return = T>
using EnableIfNotCopyConstructible = typename std::enable_if_t<
                                            !std::is_copy_constructible_v<T>, Return>;


template <typename T>
auto value_to_pointer(CleanType<T>&& src)
{
    return &src;
}

template <typename T>
auto value_to_pointer(const CleanType<T>& src)
{
    return const_cast<CleanType<T>*>(&src);
}

template <typename T>
auto value_to_pointer(const CleanType<T>* src)
{
    return const_cast<CleanType<T>*>(src);
}

template <typename T>
EnableIfPointer<T> pointer_to_value(CleanType<T>* src)
{
    return src;
}

template <typename T>
EnableIfReference<T> pointer_to_value(CleanType<T>* src)
{
    return static_cast<T>(*src);
}

} // namespace detail
} // namespace py
