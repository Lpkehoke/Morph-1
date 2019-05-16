#pragma once

#include "python/function.h"
#include "python/instance.h"
#include "python/internals.h"
#include "python/pythonapi.h"

#include <stdexcept>

namespace py
{

template <typename Class>
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

        detail::internals().register_type<Class>(type);
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
