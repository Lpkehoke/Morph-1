#pragma once

namespace foundation
{
namespace heterogeneous
{
namespace detail
{

constexpr auto no_op_delete = [](void*){};

template <typename T>
constexpr auto default_delete = [](void* value)
{
    delete reinterpret_cast<T*>(value);
};

} // namespace detail
} // namespace heterogeneous
} // namespace foundation