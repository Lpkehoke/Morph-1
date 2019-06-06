#pragma once

#include "python/function.h"
#include "python/internals.h"
#include "python/pythonapi.h"

#include <stdexcept>

namespace py
{

template <typename Derived>
class trampoline
{
  protected:
    template <typename... Args>
    handle invoke_python_impl(const char* method_name, Args&&... args)
    {
        auto py_obj = detail::get_registered_instance<Derived>(static_cast<Derived*>(this));
        
        if (!py_obj)
        {
            throw std::runtime_error("Failed to find python instance for trampoline.");
        }

        auto meth = py_obj.getattr(method_name);
        if (!meth)
        {
            throw std::runtime_error("Python instance don't have function specified.");
        }

        auto res = meth(std::forward<Args>(args)...);
        if (!res)
        {
            auto str = PyUnicode_AsUTF8(PyObject_Str(meth.ptr()));
            throw std::runtime_error(str);
        }

        return res;
    }
};

} // namespace py
