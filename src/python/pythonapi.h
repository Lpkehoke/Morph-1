#pragma once

#include <Python.h>

#include <functional>
#include <stdexcept>
#include <string>

namespace py
{

class type_object;

class Handle
{
  public:
    Handle();
    Handle(PyObject* py_obj);
    Handle(const Handle& other);
    Handle(Handle&& other);

    PyObject* ptr();

    Handle& inc_ref();
    Handle& dec_ref();

    type_object type() const;

    void setattr(const char* name, Handle py_obj);
    Handle getattr(const char* name);

    template <typename... Args>
    Handle operator()(Args&&... args);

    bool is(Handle other) const;
    std::string repr() const;

    operator bool() const;

  protected:
   PyObject* m_ptr;
};

class Object : public Handle
{
  public:
    Object();
    Object(PyObject* py_obj);
    Object(const Object& other);
    Object(Object&& other);
    ~Object();

    Object& operator=(const Object& other);

    Object& inc_ref();
    Object& dec_ref();

    Handle release();

    operator bool() const;
};

class Str : public Object
{
};

class Tuple : public Object
{
  public:
    Tuple();

    Tuple(PyObject* py_obj);

    Handle operator[](std::size_t idx) const;

    std::size_t size() const;
};

class type_object : public Object
{
  public:
    type_object();
    type_object(PyObject* py_type_obj);
    type_object(PyTypeObject* py_type_obj);
    type_object(const type_object& other);
    type_object(type_object&& other);

    type_object& operator=(const type_object& other);
    
    PyTypeObject* type_ptr();

    Tuple mro();
};

class List : public Object
{
};

class Dict : public Object
{
};

class Capsule : public Object
{
  public:
    template <typename T, typename Dtor>
    Capsule(T* ptr, Dtor dtor)
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
