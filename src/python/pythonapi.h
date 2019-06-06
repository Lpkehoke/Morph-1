#pragma once

#include <Python.h>

#include <functional>
#include <stdexcept>
#include <string>

namespace py
{

class type_object;

class handle
{
  public:
    handle();
    handle(PyObject* py_obj);
    handle(const handle& other);
    handle(handle&& other);

    PyObject* ptr();

    handle& inc_ref();
    handle& dec_ref();

    type_object type() const;

    void setattr(const char* name, handle py_obj);
    handle getattr(const char* name);

    template <typename... Args>
    handle operator()(Args&&... args);

    bool is(handle other) const;
    std::string repr() const;

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
    object(object&& other);
    ~object();

    object& operator=(const object& other);

    object& inc_ref();
    object& dec_ref();

    handle release();

    operator bool() const;
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

    std::size_t size() const;
};

class type_object : public object
{
  public:
    type_object();
    type_object(PyObject* py_type_obj);
    type_object(PyTypeObject* py_type_obj);
    type_object(const type_object& other);
    type_object(type_object&& other);

    type_object& operator=(const type_object& other);
    
    PyTypeObject* type_ptr();

    tuple mro();
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
