#pragma once

#include "python/class.h"
#include "python/instance.h"
#include "python/internals.h"
#include "python/pythonapi.h"

#include <string>
#include <type_traits>

namespace py
{

template <typename T>
struct type_caster
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

    static handle cast(T src)
    {
        throw std::logic_error("Not implemented.");
    }
};

template<>
struct type_caster<int>
{
    static int load(handle from)
    {
        if (PyLong_Check(from.ptr()))
        {
            return PyLong_AsLong(from.ptr());
        }

        throw std::runtime_error("Failed to load value.");
    }

    static handle cast(int src)
    {
        return PyLong_FromLong(src);
    }
};

} // namespace py
