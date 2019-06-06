#pragma once

#include "python/pythonapi.h"

#include <unordered_map>
#include <memory>
#include <typeindex>

namespace py
{
namespace detail
{

struct instance
{
    using held_t = std::unordered_map<std::type_index, std::shared_ptr<void>>;
    PyObject_HEAD

    held_t m_held;
};


handle make_new_instance(PyTypeObject* subtype);
type_object make_new_base_class();
type_object make_new_type(const char* name, object nmspace);
type_object make_abstract_method_type();
object make_abstract_method_instance(const char* name);

} // namespace detail
} // namespace py
