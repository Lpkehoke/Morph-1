#pragma once

#include "python/class.h"
#include "python/exceptions.h"
#include "python/instance.h"
#include "python/internals.h"
#include "python/pythonapi.h"
#include "python/utils.h"

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>


namespace py
{

namespace detail
{

template <typename T>
enable_if_copy_constructible<T*> copy(T* src)
{
    return new T(static_cast<const T&>(*src));
}

template <typename T>
enable_if_not_copy_constructible<T*> copy(T* src)
{
    throw std::runtime_error("Can't copy type with missing copy constructor.");
}

template <typename T>
T* move(T* src)
{
    return new T(std::move(*src));
}

} // namespace detail

enum class return_value_policy
{
    copy,
    move,
    reference
};

template <typename T, typename = void>
struct loader
{
    using this_t = typename std::remove_pointer_t<typename std::decay_t<T>>;

    static T load(handle from)
    {
        this_t* res = load_impl(from);
        return detail::pointer_to_value<T>(res);
    }

    static this_t* load_impl(handle from)
    {
        if (PyObject_TypeCheck(
                from.ptr(),
                detail::internals().type_info_for_<this_t>().type_ptr()))
        {
            auto inst = reinterpret_cast<detail::instance*>(from.ptr());

            if (!inst->m_holder)
            {
                auto tmp = new typename std::aligned_storage<sizeof(this_t), alignof(this_t)>::type();
                auto this_ptr = reinterpret_cast<this_t*>(tmp);

                inst->m_holder = new detail::holder_t(this_ptr, true);

                detail::internals().register_instance(this_ptr, from);
                return this_ptr;
            }

            auto holder = inst->m_holder;
            while (true)
            {
                if (holder->m_held_tinfo->hash_code() == typeid(this_t).hash_code())
                {
                    auto this_ptr = reinterpret_cast<this_t*>(holder->m_held.get());
                    return this_ptr;
                }

                if (!holder->m_next)
                {
                    auto tmp = new typename std::aligned_storage<sizeof(this_t), alignof(this_t)>::type();
                    auto this_ptr = reinterpret_cast<this_t*>(tmp);

                    holder->m_next = new detail::holder_t(this_ptr, true);
                    detail::internals().register_instance(this_ptr, from);
                    return this_ptr;
                }

                holder = holder->m_next;
            }
        }

        throw load_error();
    }
};


template <typename T, typename = void>
struct caster
{
    using this_t = detail::clean_t<T>;

    static handle cast(T src, return_value_policy ret_val_policy)
    {
        this_t* this_ptr = detail::value_to_pointer<T>(src);

        auto py_obj = detail::internals().object_for_<this_t>(this_ptr);
        auto type = detail::internals().type_info_for_<this_t>();

        if (py_obj)
        {
            assert(PyObject_TypeCheck(py_obj.ptr(), type.type_ptr()));
            return py_obj.inc_ref();
        }
        
        auto res = detail::make_new_instance(type.type_ptr());

        if (!res)
        {
            return res;
        }

        auto inst = reinterpret_cast<detail::instance*>(res.ptr());
        this_t* payload = nullptr;
        bool is_owned = false;

        switch (ret_val_policy)
        {
          case return_value_policy::copy:
            payload = detail::copy(this_ptr);
            is_owned = true;
            break;

          case return_value_policy::move:
            payload = detail::move(this_ptr);
            is_owned = true;
            break;

          case return_value_policy::reference:
            payload = this_ptr;
            break;

          default:
            assert(false && "Unhandled return policy.");
        }

        if (!inst->m_holder)
        {
            inst->m_holder = new detail::holder_t(payload, is_owned);
        }
        else
        {
            auto holder = inst->m_holder;
            while (holder->m_next)
            {
                holder = holder->m_next;
            }

            holder->m_next = new detail::holder_t(payload, is_owned);
        }

        detail::internals().register_instance(payload, res);

        return res;
    }
};


//
//  Integral types specialization.
//

template<typename T>
struct caster<T, typename std::enable_if<std::is_integral<T>::value>::type>
{
    static handle cast(T src, return_value_policy)
    {
        return PyLong_FromLongLong(src);
    }
};

template<typename T>
struct loader<T, typename std::enable_if<std::is_integral<T>::value>::type>
{
    static T load(handle from)
    {
        if (PyLong_Check(from.ptr()))
        {
            return static_cast<T>(PyLong_AsLongLong(from.ptr()));
        }

        throw load_error {};
    }
};


//
//  std::string specialization.
//

template<>
struct caster<std::string>
{
    static handle cast(const std::string& src, return_value_policy)
    {
        return PyUnicode_FromString(src.c_str());
    }
};

template<>
struct loader<std::string>
{
    static std::string load(handle from)
    {
        if (PyUnicode_Check(from.ptr()))
        {
            return PyUnicode_AsUTF8(from.ptr());
        }

        throw load_error {};
    }
};


//
//  bool specialization.
//

template<>
struct caster<bool>
{
    static handle cast(bool src, return_value_policy)
    {
        return PyBool_FromLong(static_cast<long>(src));
    }
};

template<>
struct loader<bool>
{
    static bool load(handle from)
    {
        if (PyBool_Check(from.ptr()))
        {
            return static_cast<bool>(PyLong_AsLong(from.ptr()));
        }

        throw load_error {};
    }
};

} // namespace py
