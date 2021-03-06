#pragma once

#include "python/deleter.h"
#include "python/exceptions.h"
#include "python/instance.h"
#include "python/internals.h"
#include "python/pythonapi.h"
#include "python/utils.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <type_traits>


namespace py
{
namespace detail
{


template <typename T>
EnableIfCopyConstructible<T, T*> copy(T* src)
{
    return new T(static_cast<const T&>(*src));
}


template <typename T>
EnableIfNotCopyConstructible<T, T*> copy(T* src)
{
    throw std::runtime_error("Can't copy type with missing copy constructor.");
}


template <typename T>
T* move(T* src)
{
    return new T(std::move(*src));
}


template <typename T>
std::shared_ptr<void> make_uninitialized_value()
{
    auto this_ptr = new typename std::aligned_storage<sizeof(T), alignof(T)>::type();
    
    return std::shared_ptr<void>(
        static_cast<void*>(this_ptr),
        detail::default_delete<T>);
}


template <typename T>
std::shared_ptr<T> load_value(Handle src)
{
    if (!src)
    {
        return std::shared_ptr<T>();
    }

    auto base = internals().base_class();

    if (!PyObject_IsSubclass(PyObject_Type(src.ptr()), base.ptr()))
    {
        return std::shared_ptr<T>();
    }

    auto mro = src.type().mro();
    auto mro_size = mro.size();

    const std::type_info* cpp_type_to_load = nullptr;
    bool exact_cpp_type = false;

    for (std::size_t i = 0; i < mro_size; ++i)
    {
        auto py_tinfo = get_python_type_info(mro[i].ptr());
        
        if (!py_tinfo)
        {
            continue;
        }

        if (py_tinfo->m_cpp_tinfo->m_tinfo->hash_code() == typeid(T).hash_code())
        {
            cpp_type_to_load = py_tinfo->m_cpp_tinfo->m_tinfo;
            exact_cpp_type = true;
            break;
        }

        for (const auto type_idx : py_tinfo->m_conversions)
        {
            if (type_idx.hash_code() == typeid(T).hash_code())
            {
                cpp_type_to_load = py_tinfo->m_cpp_tinfo->m_tinfo;
                break;
            }
        }

        if (cpp_type_to_load) break;
    }

    mro.release();

    if (!cpp_type_to_load)
    {
        throw std::runtime_error("Can't load the value.");
    }

    auto inst = reinterpret_cast<detail::Instance*>(src.ptr());

    auto itr = inst->m_held.find(*cpp_type_to_load);
    if (itr != inst->m_held.end())
    {
        return std::reinterpret_pointer_cast<T>(itr->second);
    }

    if (!exact_cpp_type)
    {
        throw std::runtime_error("Can't load uninitialized value.");
    }

    auto this_ptr = make_uninitialized_value<T>();
    detail::register_instance(this_ptr.get(), inst);

    auto res = std::reinterpret_pointer_cast<T>(this_ptr);

    inst->m_held.emplace(typeid(T), std::move(this_ptr));

    return res;
}

} // namespace detail

enum class return_value_policy
{
    copy,
    move,
    reference
};

template <typename T, typename = void>
struct Loader
{
    using ThisType = detail::CleanType<T>;

    static T load(Handle from)
    {
        auto res = detail::load_value<ThisType>(from);

        if (!res)
        {
            throw LoadError();
        }

        return detail::pointer_to_value<T>(res.get());
    }
};


template <typename T>
struct Loader<std::shared_ptr<T>>
{
    using ThisType = detail::CleanType<T>;

    static std::shared_ptr<T> load(Handle from)
    {
        auto res = detail::load_value<ThisType>(from);

        if (!res)
        {
            throw LoadError();
        }

        return res;
    }
};


template <typename T, typename = void>
struct Caster
{
    static Handle cast(T* src, return_value_policy ret_val_policy)
    {
        auto tinfo = detail::get_python_type_info<T>();
        if (!tinfo)
        {
            throw std::runtime_error("No registered python type to cast to.");
        }

        auto py_obj = detail::get_registered_instance<T>(src);
        auto type = tinfo->m_py_type;

        if (py_obj)
        {
            if (!PyObject_TypeCheck(py_obj.ptr(), type.type_ptr()))
            {
                // TODO: make a better error message.
                auto str = PyObject_Str(py_obj.ptr());
                auto cstr = PyUnicode_AsUTF8(str);
                throw std::runtime_error(std::string("Type: ") + type.type_ptr()->tp_name + " vs " + cstr);
            }
            return py_obj.inc_ref();
        }
        
        auto res = detail::make_new_instance(type.type_ptr());

        if (!res)
        {
            return nullptr;
        }

        auto inst = reinterpret_cast<detail::Instance*>(res.ptr());
        T* payload = nullptr;
        bool take_ownership = false;

        switch (ret_val_policy)
        {
          case return_value_policy::copy:
            take_ownership = true;
            payload = detail::copy(src);
            break;

          case return_value_policy::move:
            take_ownership = true;
            payload = detail::move(src);
            break;

          case return_value_policy::reference:
            payload = src;
            break;

          default:
            assert(false && "Unhandled return policy.");
        }

        auto this_ptr = std::shared_ptr<void>(
            static_cast<void*>(payload),
            take_ownership
                ? detail::default_delete<T>
                : detail::no_op_delete);

        inst->m_held.emplace(typeid(T), std::move(this_ptr));

        detail::register_instance(payload, inst);

        return res;
    }
};

//
//  Integral types specialization.
//

template<typename T>
struct Caster<T, typename std::enable_if_t<std::is_integral<T>::value>>
{
    static Handle cast(T* src, return_value_policy)
    {
        return PyLong_FromLongLong(*src);
    }
};

template<typename T>
struct Loader<T, typename std::enable_if_t<std::is_integral<T>::value>>
{
    static T load(Handle from)
    {
        if (PyLong_Check(from.ptr()))
        {
            return static_cast<T>(PyLong_AsLongLong(from.ptr()));
        }

        throw LoadError {};
    }
};


//
//  std::string specialization.
//

template<typename T>
struct Caster<T,
              typename std::enable_if_t<
                    std::is_same_v<
                        typename std::remove_reference_t<T>,
                        std::string>>>
{
    static Handle cast(std::string* src, return_value_policy)
    {
        return PyUnicode_FromString(src->c_str());
    }
};

template<>
struct Loader<std::string>
{
    static std::string load(Handle from)
    {
        if (PyUnicode_Check(from.ptr()))
        {
            return PyUnicode_AsUTF8(from.ptr());
        }

        throw LoadError {};
    }
};


//
//  bool specialization.
//

template<>
struct Caster<bool>
{
    static Handle cast(bool* src, return_value_policy)
    {
        Handle res = *src ? Py_True : Py_False;
        return res.inc_ref();
    }
};

template<>
struct Loader<bool>
{
    static bool load(Handle from)
    {
        if (PyBool_Check(from.ptr()))
        {
            return static_cast<bool>(PyLong_AsLong(from.ptr()));
        }

        throw LoadError {};
    }
};

template <typename T>
Handle cast(T&& src, return_value_policy ret_val_policy)
{
    using ThisType = detail::CleanType<T>;

    ThisType* src_ptr = detail::value_to_pointer<T>(src);
    return Caster<ThisType>::cast(src_ptr, ret_val_policy);
}

} // namespace py
