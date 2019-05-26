#pragma once

#include <type_traits>
#include <typeinfo>


namespace foundation
{
namespace heterogeneous
{
namespace detail
{

struct type_info
{
    using ctor_t = void* (void*);

    type_info(
        const std::type_info*   tinfo,
        ctor_t*                 copy_ctor,
        ctor_t*                 move_ctor)
      : m_tinfo(tinfo)
      , m_copy_ctor(copy_ctor)
      , m_move_ctor(move_ctor)
    {}

    const std::type_info*   m_tinfo;
    ctor_t*                 m_copy_ctor;
    ctor_t*                 m_move_ctor;
};


template <typename T, typename = void>
struct copy_ctor_for_
{
    static constexpr type_info::ctor_t* value = nullptr;
};

template <typename T>
struct copy_ctor_for_<T, typename std::enable_if_t<std::is_copy_constructible_v<T>>>
{
    static constexpr type_info::ctor_t* value = [](void* other_ptr)
        {
            auto this_ptr_typed = reinterpret_cast<T*>(other_ptr);
            auto res = new T(static_cast<const T&>(*this_ptr_typed));;
            return static_cast<void*>(res);
        };
};

template <typename T>
struct move_ctor_for_
{
    static constexpr type_info::ctor_t* value = [](void* other_ptr)
        {
            auto this_ptr_typed = reinterpret_cast<T*>(other_ptr);
            auto res = new T(static_cast<T&&>(*this_ptr_typed));
            return static_cast<void*>(res);
        };
};


template <typename T>
static const type_info type_info_for_ = type_info(
    &typeid(T),
    copy_ctor_for_<T>::value,
    move_ctor_for_<T>::value
);

} // namespace detail
} // namespace heterogeneous
} // namespace foundation
