#include "python/pythonapi.h"

#include "python/exceptions.h"

#include <stdexcept>
#include <utility>

namespace py
{

handle::handle()
  : m_ptr(nullptr)
{}

handle::handle(PyObject* py_obj)
  : m_ptr(py_obj)
{}

handle::handle(const handle& other)
  : m_ptr(other.m_ptr)
{}

handle::handle(handle&& other)
  : m_ptr(nullptr)
{
    std::swap(m_ptr, other.m_ptr);
}

PyObject* handle::ptr()
{
    return m_ptr;
}

handle& handle::inc_ref()
{
    Py_XINCREF(m_ptr);
    return *this;
}

handle& handle::dec_ref()
{
    Py_XDECREF(m_ptr);
    return *this;
}

type_object handle::type() const
{
    return PyObject_Type(m_ptr);
}

void handle::setattr(const char* name, handle py_obj)
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

handle handle::getattr(const char* name)
{
    if (m_ptr)
    {
        auto res = PyObject_GetAttrString(m_ptr, name);

        if (!res)
        {
            throw error_already_set();
        }

        return res;
    }
    else
    {
        throw std::runtime_error("Trying to get attribute of invalid handle.");
    }
}

bool handle::is(handle other) const
{
    return m_ptr == other.m_ptr;
}

std::string handle::repr() const
{
    auto py_str = PyObject_Repr(m_ptr);
    auto res = PyUnicode_AsUTF8(py_str);
    Py_XDECREF(py_str);

    return res;
}

handle::operator bool() const
{
    return m_ptr != nullptr;
}

object::object()
  : handle()
{}

object::object(PyObject* py_obj)
  : handle(py_obj)
{}

object::object(const object& other)
  : handle(other)
{
    inc_ref();
}

object::object(object&& other)
  : handle(std::move(other))
{}

object::~object()
{
    dec_ref();
}

object& object::operator=(const object& other)
{
    if (!is(other))
    {
        dec_ref();
        m_ptr = other.m_ptr;
        inc_ref();
    }

    return *this;
}

object& object::inc_ref()
{
    Py_XINCREF(m_ptr);
    return *this;
}

object& object::dec_ref()
{
    Py_XDECREF(m_ptr);
    return *this;
}

handle object::release()
{
    handle res(m_ptr);
    m_ptr = nullptr;
    return res;
}

object::operator bool() const
{
    return m_ptr != nullptr;
}

type_object::type_object()
  : object()
{}

type_object::type_object(PyObject* py_type_obj)
  : object(py_type_obj)
{}

type_object::type_object(PyTypeObject* py_type_obj)
  : object(reinterpret_cast<PyObject*>(py_type_obj))
{}

type_object::type_object(const type_object& other)
  : object(other)
{}

type_object::type_object(type_object&& other)
  : object(std::move(other))
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

tuple type_object::mro()
{
    return type_ptr()->tp_mro;
}

tuple::tuple()
  : object()
{}

tuple::tuple(PyObject* py_obj)
  : object(py_obj)
{
    if (!PyTuple_Check(py_obj))
    {
        throw std::runtime_error(
            "Trying to initialize py::tuple with a pointer to non-tuple object.");
    }
}

handle tuple::operator[](std::size_t idx) const
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

std::size_t tuple::size() const
{
    if (m_ptr)
    {
        return PyTuple_Size(m_ptr);
    }
    
    throw std::runtime_error("Trying to get size of invalid tuple object.");
}

} // namespace py
