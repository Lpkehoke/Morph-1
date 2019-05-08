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

template <typename T>
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

template <typename T>
struct caster
{
    using this_t = typename std::decay<T>::type;

    static handle cast(T src)
    {
        return cast_impl(std::forward<T>(src));
    }

    /**
     *  Python wrapper will reference existing cpp object.
     *  Note: deallocation of associated memory will crash the program.
     */
    static handle cast_impl(this_t* src)
    {
        auto py_obj = detail::internals().object_for_<this_t>(src);

        if (py_obj)
        {
            return py_obj;
        }

        auto type = detail::internals().type_info_for_<this_t>();
        auto res = type.create_instance();

        auto inst = reinterpret_cast<detail::instance*>(res.ptr());
        inst->m_held.emplace(typeid(this_t), static_cast<void*>(src));

        return res.release();
    }

    /**
     *  Python wrapper will reference existing cpp object.
     *  Note: deallocation of associated memory will crash the program.
     */
    static handle cast_impl(this_t& src)
    {
        return cast_impl(&src);
    }

    /** 
     *  Python wrapper will be initialized using copy constructor.
     */
    static handle cast_impl(const this_t& src)
    {
        auto py_obj = detail::internals().object_for_<this_t>(&src);

        if (py_obj)
        {
            return py_obj;
        }

        auto type = detail::internals().type_info_for_<this_t>();
        auto res = type.create_instance();

        this_t* payload = new this_t(src);

        auto inst = reinterpret_cast<detail::instance*>(res.ptr());
        inst->m_held.emplace(typeid(this_t), static_cast<void*>(payload));

        return res;
    }

    /**
     *  Python wrapper will be initialized using move constructor.
     */
    static handle cast_impl(this_t&& src)
    {
        auto py_obj = detail::internals().object_for_<this_t>(&src);

        if (py_obj)
        {
            return py_obj;
        }

        auto type = detail::internals().type_info_for_<this_t>();
        auto res = type.create_instance();

        this_t* payload = new this_t(std::move(src));

        auto inst = reinterpret_cast<detail::instance*>(res.ptr());
        inst->m_held.emplace(typeid(this_t), static_cast<void*>(payload));

        detail::internals().register_instance(payload, res);

        return res;
    }
};

template<>
struct caster<int>
{
    static handle cast(int src)
    {
        return PyLong_FromLong(src);
    }
};

template<>
struct loader<int>
{
    static int load(handle from)
    {
        if (PyLong_Check(from.ptr()))
        {
            return PyLong_AsLong(from.ptr());
        }

        throw load_error {};
    }
};

} // namespace py
