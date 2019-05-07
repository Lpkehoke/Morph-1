#pragma once

#include "exceptions.h"

#include <Python.h>

#include <functional>
#include <stdexcept>
#include <utility>

namespace py
{

class handle
{
  public:
    handle()
      : m_ptr(nullptr)
    {}

    handle(PyObject* py_obj)
      : m_ptr(py_obj)
    {}

    PyObject* ptr()
    {
        return m_ptr;
    }

    void setattr(const char* name, handle py_obj)
    {
        if (m_ptr)
        {
            PyObject_SetAttrString(m_ptr, name, py_obj.ptr());
        }
    }

  protected:
   PyObject* m_ptr;
};

class object : public handle
{
  public:
    object()
      : handle()
    {}

    object(PyObject* py_obj)
      : handle(py_obj)
    {}

    object(const object& other)
      : handle(other)
    {}

    object& inc_ref()
    {
        Py_XINCREF(m_ptr);
        return *this;
    }

    object& dec_ref()
    {
        Py_XDECREF(m_ptr);
        return *this;
    }

    handle release()
    {
        handle res(m_ptr);
        m_ptr = nullptr;
        return res;
    }

    operator bool() const
    {
        return m_ptr != nullptr;
    }
};

class type_object : public object
{
  public:
    type_object()
      : object()
    {}

    type_object(PyTypeObject* py_type_obj)
      : object(reinterpret_cast<PyObject*>(py_type_obj))
    {}

    PyTypeObject* type_ptr()
    {
        return reinterpret_cast<PyTypeObject*>(m_ptr);
    }
};

class str : public object
{
};

class tuple : public object
{
  public:
    tuple()
      : object()
    {}

    tuple(PyObject* py_obj)
      : object(py_obj)
    {
        if (!PyTuple_Check(py_obj))
        {
            throw std::runtime_error(
                "Trying to initialize py::tuple with a pointer to non-tuple object.");
        }
    }

    handle operator[] (const std::size_t idx) const
    {
        if (m_ptr)
        {
            auto ref = PyTuple_GetItem(m_ptr, idx);
            if (ref)
            {
                return handle(ref);
            }

            throw error_already_set();
        }

        return nullptr;
    }
};

class list : public object
{
};

class dict : public object
{
};

class capsule : public object
{
  public:
    template <typename T, typename Dtor>
    capsule(T* ptr, Dtor dtor)
    {
        auto dtor_fn = static_cast<void (*)(T*)>(dtor);

        m_ptr = PyCapsule_New(
            static_cast<void*>(ptr),
            nullptr,
            [](PyObject* self)
            {
                auto dtor = reinterpret_cast<void (*)(T*)>(PyCapsule_GetContext(self));
                auto ptr = reinterpret_cast<T*>(PyCapsule_GetPointer(self, nullptr));
                std::invoke(dtor, ptr);
            });

        if (!m_ptr)
        {
            throw std::runtime_error("Failed to allocate capsule.");
        }

        PyCapsule_SetContext(m_ptr, reinterpret_cast<void*>(dtor_fn));
    }
};

} // namespace py
