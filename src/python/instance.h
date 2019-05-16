#pragma once

#include "python/pythonapi.h"

#include <memory>
#include <typeinfo>
#include <unordered_map>

namespace py
{
namespace detail
{

struct holder_t
{
    using held_t = std::shared_ptr<void>;

    template <typename T>
    holder_t(T* payload, bool is_owner)
      : m_held_tinfo(&typeid(T))
      , m_next(nullptr)
      , m_held(
          payload,
          [is_owner](T* ptr)
          {
              if (is_owner)
              {
                  delete ptr;
              }
          })
    {}

    ~holder_t()
    {
        delete m_next;
    }

    // Type info for held type (i.e. pointee type).
    const std::type_info*   m_held_tinfo;

    holder_t*           m_next;
    held_t              m_held;
};

struct instance
{
    PyObject_HEAD

    holder_t*           m_holder;
    PyObject*           m_dict;
};


PyObject* make_new_instance(PyTypeObject* subtype);
type_object make_new_base_class();
type_object make_new_type(const char* name, type_object base_class);
type_object make_abstract_method_type();
object make_abstract_method_instance(const char* name);

} // namespace detail
} // namespace py
