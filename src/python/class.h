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
struct BaseClass {};

namespace detail
{

template <typename Class, typename T>
struct HandleBaseClassImpl
{
    using Conversions = detail::PythonTypeInfo::Conversions;

    static void handle(Conversions& conversions)
    {}
};

template <typename Class, typename T>
struct HandleBaseClassImpl<Class, BaseClass<T>>
{
    using Conversions = detail::PythonTypeInfo::Conversions;

    static void handle(Conversions& conversions)
    {
        static_assert(std::is_base_of_v<T, Class>);
        conversions.insert(typeid(T));
    }
};

template <typename Class, typename... Types>
struct HandleBaseClass
{
    using Conversions = detail::PythonTypeInfo::Conversions;

    static Conversions handle()
    {
        Conversions res;
        (HandleBaseClassImpl<Class, Types>::handle(res),...);
        return res;
    }
};

} // namespace detail


template <typename Class, typename... Options>
class ExposeClass
{
  public:
    ExposeClass(module scope, const char* name)
      : m_name(name)
      , m_scope(scope)
      , m_namespace(PyDict_New())
    {}

    ~ExposeClass()
    {
        auto type = detail::make_new_type(m_name, m_namespace);
        m_scope.add_object(m_name, type);

        auto conversions = detail::HandleBaseClass<Class, Options...>::handle();
        detail::register_python_type<Class>(type, std::move(conversions));
    }

    template <typename Func>
    ExposeClass& def(
        const char*         name,
        Func&&              fn,
        return_value_policy policy = return_value_policy::copy)
    {
        detail::CppFunction(name, m_namespace, policy, std::forward<Func>(fn));
        return *this;
    }

    template <typename... Args>
    ExposeClass& def(Init<Args...>)
    {
        detail::CppFunction(Init<Class, Args...> {}, m_namespace);
        return *this;
    }

    ExposeClass& def_abstract(const char* name)
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
    Object      m_namespace;
};

} // namespace py
