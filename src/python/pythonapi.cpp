#include "python/pythonapi.h"

#include "python/exceptions.h"
#include "python/instance.h"

#include <functional>
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

PyObject* handle::ptr()
{
    return m_ptr;
}

void handle::setattr(const char* name, handle py_obj)
{
    if (m_ptr)
    {
        PyObject_SetAttrString(m_ptr, name, py_obj.ptr());
    }
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
{}

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

type_object::type_object(PyTypeObject* py_type_obj)
  : object(reinterpret_cast<PyObject*>(py_type_obj))
{}

PyTypeObject* type_object::type_ptr()
{
    return reinterpret_cast<PyTypeObject*>(m_ptr);
}

object type_object::create_instance()
{
    object res = detail::make_new_instance(type_ptr());
    return res;
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

} // namespace py
