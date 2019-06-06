#pragma once

#include "python/function.h"
#include "python/instance.h"
#include "python/internals.h"
#include "python/module.h"
#include "python/pythonapi.h"

#include "foundation/heterogeneous/typeinfo.h"

#include <stdexcept>

namespace py
{

template <typename T>
struct base_class {};

namespace detail
{

template <typename Class, typename T>
struct handle_base_class_impl
{
    using type_set_t = detail::python_type_info::conversions_set_t;

    static void handle(type_set_t& conversions)
    {}
};

template <typename Class, typename T>
struct handle_base_class_impl<Class, base_class<T>>
{
    using type_set_t = detail::python_type_info::conversions_set_t;

    static void handle(type_set_t& conversions)
    {
        static_assert(std::is_base_of_v<T, Class>);
        conversions.insert(typeid(T));
    }
};

template <typename Class, typename... Types>
struct handle_base_class
{
    using type_set_t = detail::python_type_info::conversions_set_t;

    static type_set_t handle()
    {
        type_set_t res;
        (handle_base_class_impl<Class, Types>::handle(res),...);
        return res;
    }
};

} // namespace detail


template <typename Class, typename... Options>
class class_
{
  public:
    class_(module scope, const char* name)
      : m_name(name)
      , m_scope(scope)
      , m_namespace(PyDict_New())
    {}

    ~class_()
    {
        auto type = detail::make_new_type(m_name, m_namespace);
        m_scope.add_object(m_name, type);

        auto conversions = detail::handle_base_class<Class, Options...>::handle();
        detail::register_python_type<Class>(type, std::move(conversions));
    }

    template <typename Func>
    class_& def(
        const char*         name,
        Func&&              fn,
        return_value_policy policy = return_value_policy::copy)
    {
        detail::cpp_function(name, m_namespace, policy, std::forward<Func>(fn));
        return *this;
    }

    template <typename... Args>
    class_& def(init<Args...>)
    {
        detail::cpp_function(init<Class, Args...> {}, m_namespace);
        return *this;
    }

    class_& def_abstract(const char* name)
    {
        auto meth = detail::make_abstract_method_instance(name);

        if (!meth)
        {
            throw std::runtime_error("Failed to create abstract method.");
        }

        PyDict_SetItemString(m_namespace.ptr(), name, meth.ptr());

        return *this;
    }

  private:
    const char* m_name;
    module      m_scope;

    // TODO: use dict wrapper instead.
    object      m_namespace;
};

} // namespace py
