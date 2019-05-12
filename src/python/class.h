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
      : m_scope(scope)
    {
        m_type_obj = detail::make_new_type(name, detail::internals().base_class());
        scope.add_object(name, m_type_obj);

        detail::internals().register_type<Class>(m_type_obj);
    }

    template <typename Func>
    class_& def(
        const char*         name,
        Func&&              fn,
        return_value_policy policy = return_value_policy::copy)
    {
        detail::cpp_function(name, m_type_obj, policy, std::forward<Func>(fn));
        return *this;
    }

    template <typename... Args>
    class_& def(init<Args...>)
    {
        detail::cpp_function(init<Class, Args...> {}, m_type_obj);
        return *this;
    }

  private:
    module      m_scope;
    type_object m_type_obj;
};

} // namespace py
