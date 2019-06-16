#include "python/pythonapi.h"

#include "python/exceptions.h"

#include <stdexcept>
#include <utility>

namespace py
{

Handle::Handle()
  : m_ptr(nullptr)
{}

Handle::Handle(PyObject* py_obj)
  : m_ptr(py_obj)
{}

Handle::Handle(const Handle& other)
  : m_ptr(other.m_ptr)
{}

Handle::Handle(Handle&& other)
  : m_ptr(nullptr)
{
    std::swap(m_ptr, other.m_ptr);
}

PyObject* Handle::ptr()
{
    return m_ptr;
}

Handle& Handle::inc_ref()
{
    Py_XINCREF(m_ptr);
    return *this;
}

Handle& Handle::dec_ref()
{
    Py_XDECREF(m_ptr);
    return *this;
}

type_object Handle::type() const
{
    return PyObject_Type(m_ptr);
}

void Handle::setattr(const char* name, Handle py_obj)
{
    if (m_ptr)
    {
        PyObject_SetAttrString(m_ptr, name, py_obj.ptr());
    }
    else
    {
        throw std::runtime_error("Trying to set attribute of invalid handle.");
    }
}

Handle Handle::getattr(const char* name)
{
    if (m_ptr)
    {
        auto res = PyObject_GetAttrString(m_ptr, name);

        if (!res)
        {
            throw ErrorAlreadySet();
        }

        return res;
    }
    else
    {
        throw std::runtime_error("Trying to get attribute of invalid handle.");
    }
}

bool Handle::is(Handle other) const
{
    return m_ptr == other.m_ptr;
}

std::string Handle::repr() const
{
    auto py_str = PyObject_Repr(m_ptr);
    auto res = PyUnicode_AsUTF8(py_str);
    Py_XDECREF(py_str);

    return res;
}

Handle::operator bool() const
{
    return m_ptr != nullptr;
}

Object::Object()
  : Handle()
{}

Object::Object(PyObject* py_obj)
  : Handle(py_obj)
{}

Object::Object(const Object& other)
  : Handle(other)
{
    inc_ref();
}

Object::Object(Object&& other)
  : Handle(std::move(other))
{}

Object::~Object()
{
    dec_ref();
}

Object& Object::operator=(const Object& other)
{
    if (!is(other))
    {
        dec_ref();
        m_ptr = other.m_ptr;
        inc_ref();
    }

    return *this;
}

Object& Object::inc_ref()
{
    Py_XINCREF(m_ptr);
    return *this;
}

Object& Object::dec_ref()
{
    Py_XDECREF(m_ptr);
    return *this;
}

Handle Object::release()
{
    Handle res(m_ptr);
    m_ptr = nullptr;
    return res;
}

Object::operator bool() const
{
    return m_ptr != nullptr;
}

type_object::type_object()
  : Object()
{}

type_object::type_object(PyObject* py_type_obj)
  : Object(py_type_obj)
{}

type_object::type_object(PyTypeObject* py_type_obj)
  : Object(reinterpret_cast<PyObject*>(py_type_obj))
{}

type_object::type_object(const type_object& other)
  : Object(other)
{}

type_object::type_object(type_object&& other)
  : Object(std::move(other))
{}

type_object& type_object::operator=(const type_object& other)
{
    if (!is(other))
    {
        dec_ref();
        m_ptr = other.m_ptr;
        inc_ref();
    }

    return *this;
}

PyTypeObject* type_object::type_ptr()
{
    return reinterpret_cast<PyTypeObject*>(m_ptr);
}

Tuple type_object::mro()
{
    return type_ptr()->tp_mro;
}

Tuple::Tuple()
  : Object()
{}

Tuple::Tuple(PyObject* py_obj)
  : Object(py_obj)
{
    if (!PyTuple_Check(py_obj))
    {
        throw std::runtime_error(
            "Trying to initialize py::tuple with a pointer to non-tuple object.");
    }
}

Handle Tuple::operator[](std::size_t idx) const
{
    if (m_ptr)
    {
        auto ref = PyTuple_GetItem(m_ptr, idx);
        if (ref)
        {
            return Handle(ref);
        }

        throw ErrorAlreadySet();
    }

    return nullptr;
}

std::size_t Tuple::size() const
{
    if (m_ptr)
    {
        return PyTuple_Size(m_ptr);
    }
    
    throw std::runtime_error("Trying to get size of invalid tuple object.");
}

} // namespace py
