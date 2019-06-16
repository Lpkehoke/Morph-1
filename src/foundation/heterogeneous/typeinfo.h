#pragma once

#include <type_traits>
#include <typeinfo>
#include <unordered_set>


namespace foundation
{
namespace heterogeneous
{

struct TypeInfo
{
    using Ctor = void* (void*);

    TypeInfo(
        const std::type_info*   tinfo,
        Ctor*                   copy_ctor,
        Ctor*                   move_ctor)
      : m_tinfo(tinfo)
      , m_copy_ctor(copy_ctor)
      , m_move_ctor(move_ctor)
    {}

    const std::type_info*   m_tinfo;
    Ctor*                   m_copy_ctor;
    Ctor*                   m_move_ctor;
};


template <typename T, typename = void>
struct CopyCtorFor
{
    static constexpr TypeInfo::Ctor* value = nullptr;
};

template <typename T>
struct CopyCtorFor<T, typename std::enable_if_t<std::is_copy_constructible_v<T>>>
{
    static constexpr TypeInfo::Ctor* value = [](void* other_ptr)
        {
            auto this_ptr_typed = reinterpret_cast<T*>(other_ptr);
            auto res = new T(static_cast<const T&>(*this_ptr_typed));;
            return static_cast<void*>(res);
        };
};

template <typename T, typename = void>
struct MoveCtorFor
{
    static constexpr TypeInfo::Ctor* value = nullptr;
};

template <typename T>
struct MoveCtorFor<T, typename std::enable_if_t<std::is_move_constructible_v<T>>>
{
    static constexpr TypeInfo::Ctor* value = [](void* other_ptr)
        {
            auto this_ptr_typed = reinterpret_cast<T*>(other_ptr);
            auto res = new T(static_cast<T&&>(*this_ptr_typed));
            return static_cast<void*>(res);
        };
};

template <typename T>
static const auto type_info_for = TypeInfo(
    &typeid(T),
    CopyCtorFor<T>::value,
    MoveCtorFor<T>::value
);

} // namespace heterogeneous
} // namespace foundation
