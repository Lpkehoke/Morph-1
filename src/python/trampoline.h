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
        auto py_obj = detail::internals().object_for_<Derived>(static_cast<Derived*>(this));
        
        if (!py_obj)
        {
            throw std::runtime_error("Failed to find python instance for trampoline.");
        }

        auto meth = py_obj.getattr(method_name);
        if (!meth)
        {
            throw std::runtime_error("Python instance don't have function specified.");
        }

        tuple py_args = detail::cpp_to_python_tuple(std::tuple {args...});

        auto res = meth(py_args);
        if (!res)
        {
            auto str = PyUnicode_AsUTF8(PyObject_Str(meth.ptr()));
            throw std::runtime_error(str);
        }

        return res;
    }
};

} // namespace py
