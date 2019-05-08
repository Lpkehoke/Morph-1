#pragma once

#include <Python.h>

#include <functional>
#include <stdexcept>
#include <utility>

namespace py
{

class handle
{
  public:
    handle();
    handle(PyObject* py_obj);

    PyObject* ptr();

    void setattr(const char* name, handle py_obj);

    operator bool() const;

  protected:
   PyObject* m_ptr;
};

class object : public handle
{
  public:
    object();
    object(PyObject* py_obj);
    object(const object& other);

    object& inc_ref();
    object& dec_ref();

    handle release();

    operator bool() const;
};

class type_object : public object
{
  public:
    type_object();
    type_object(PyTypeObject* py_type_obj);
    
    PyTypeObject* type_ptr();

    object create_instance();
};

class str : public object
{
};

class tuple : public object
{
  public:
    tuple();

    tuple(PyObject* py_obj);

    handle operator[](std::size_t idx) const;
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
