#pragma once

#include "python/pythonapi.h"

#include <typeinfo>
#include <unordered_map>
#include <typeindex>

namespace py
{
namespace detail
{

struct instance
{
    using held_t = std::unordered_map<std::type_index, void*>;

    PyObject_HEAD

    held_t      m_held;
    PyObject*   m_dict;
};


PyObject* make_new_instance(PyTypeObject* subtype);
type_object make_new_base_class();
type_object make_new_type(const char* name, type_object base_class);

} // namespace detail
} // namespace py
