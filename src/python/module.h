#pragma once

#include "python/pythonapi.h"

#include "foundation/platform.h"

#include <stdexcept>
#include <cstring>

#define MORPH_PYTHON_MODULE(name, var, doc)             \
extern "C"                                              \
{                                                       \
void module_ ## name ## _init(py::module& var);         \
PyObject* PyInit_ ## name()                             \
{                                                       \
    auto var = py::module(#name, #doc);                 \
    module_ ## name ## _init(var);                      \
    return var.ptr();                                   \
}                                                       \
}                                                       \
void module_ ## name ## _init(py::module& var)

namespace py
{

class module : public object
{
  public:
    module(const char* name, const char* doc = nullptr)
    {
        auto def = new PyModuleDef();
        std::memset(def, 0, sizeof(PyModuleDef));
        def->m_name = name;
        def->m_doc = doc;
        def->m_size = -1;

        m_ptr = PyModule_Create(def);

        if (m_ptr == nullptr)
            throw std::runtime_error("Failed to initialize module.");

        inc_ref();
    }

    module(const module& other)
      : object(other)
    {}

    void add_object(const char* name, object py_obj)
    {
        PyModule_AddObject(m_ptr, name, py_obj.release().ptr());
    }
};

} // namespace py
