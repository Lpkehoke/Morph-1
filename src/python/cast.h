#pragma once

#include "python/class.h"
#include "python/exceptions.h"
#include "python/instance.h"
#include "python/internals.h"
#include "python/pythonapi.h"

#include <string>
#include <type_traits>

#include <iostream>

namespace py
{

namespace detail
{

template <typename T>
struct get_pointer
{
    static T* value(T* src)
    {
        return src;
    }

    static T* value(const T& src)
    {
        return &const_cast<T&>(src);
    }
};

template <typename T, typename = void>
struct copy_or_move_impl {};

template <typename T>
struct copy_or_move_impl<T, typename std::enable_if<std::is_copy_constructible<T>::value>::type>
{
    static T* value(const T& src)
    {
        return new T(src);
    }

    static T* value(T&& src)
    {
        return new T(std::forward<T>(src));
    }
};

template <typename T>
struct copy_or_move_impl<T, typename std::enable_if<!std::is_copy_constructible<T>::value>::type>
{
    static T* value(const T& src)
    {
        throw std::runtime_error("Can't copy return value. Type is not copy constructible.");
    }

    static T* value(T&& src)
    {
        return new T(std::forward<T>(src));
    }
};

template <typename T>
static T* copy_or_move(const T& src)
{
    return copy_or_move_impl<T>::value(static_cast<const T&>(src));
}

template <typename T>
static T* copy_or_move(T&& src)
{
    return copy_or_move_impl<T>::value(std::forward<T>(src));
}

template <typename T>
static T* copy_or_move(T* src)
{
    return copy_or_move_impl<T>::value(static_cast<const T&>(*src));
}

template <typename T>
static T* move(T& src)
{
    return new T(std::move(src));
}

template <typename T>
static T* move(T&& src)
{
    return new T(std::move(src));
}

template <typename T>
static T* move(T* src)
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
    using this_t = typename std::decay<T>::type;

    static T load(handle from)
    {
        if (PyObject_TypeCheck(
                from.ptr(),
                detail::internals().base_class().type_ptr()))
        {
            auto inst = reinterpret_cast<detail::instance*>(from.ptr());

            this_t* this_ptr;

            auto itr = inst->m_held.find(typeid(this_t));
            if (itr == inst->m_held.end())
            {
                this_ptr = new this_t();
                inst->m_held.emplace(typeid(this_t), static_cast<void*>(this_ptr));
            }
            else
            {
                this_ptr = reinterpret_cast<this_t*>(itr->second);
            }

            return *this_ptr;
        }

        throw std::logic_error("Not implemented.");
    }
};

template <typename T, typename = void>
struct caster
{
    using this_t = typename std::decay<T>::type;

    static handle cast(T src, return_value_policy ret_val_policy)
    {
        this_t* this_ptr = detail::get_pointer<this_t>::value(src);

        auto py_obj = detail::internals().object_for_<this_t>(this_ptr);
        if (py_obj)
        {
            return py_obj;
        }

        auto type = detail::internals().type_info_for_<this_t>();
        auto res = type.create_instance();

        auto inst = reinterpret_cast<detail::instance*>(res.ptr());
        this_t* payload = nullptr;

        switch (ret_val_policy)
        {
          case return_value_policy::copy:
            payload = detail::copy_or_move(src);
            break;

          case return_value_policy::move:
            payload = detail::move(src);
            break;

          case return_value_policy::reference:
            payload = this_ptr;
            break;
        }

        inst->m_held.emplace(typeid(this_t), static_cast<void*>(payload));
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

} // namespace py
