#include "python/instance.h"

#include "python/pythonapi.h"
#include "python/internals.h"

#include "foundation/heterogeneous/box.h"

#include <memory>
#include <stdexcept>

namespace py
{
namespace detail
{

void dealloc_instance(PyObject* self)
{
    auto inst = reinterpret_cast<Instance*>(self);

    unregister_instance(inst);
    
    std::destroy_at(&inst->m_held);

    auto type = Py_TYPE(self);
    type->tp_free(self);
}

PyObject* new_instance(PyTypeObject* subtype, PyObject*, PyObject*)
{
    auto res = PyBaseObject_Type.tp_new(subtype, PyTuple_New(0), nullptr);

    if (res)
    {
        auto inst = reinterpret_cast<Instance*>(res);
        new (&inst->m_held) Instance::Held();
    }

    return res;
}


Handle make_new_instance(PyTypeObject* subtype)
{
    return new_instance(subtype, nullptr, nullptr);
}


int init_instance(PyObject* self , PyObject*, PyObject*)
{
    PyTypeObject *type = Py_TYPE(self);
    PyErr_Format(
        PyExc_TypeError,
        "%s has no constructor defined.",
        type->tp_name);
    return -1;
}


type_object make_new_base_class()
{
    constexpr auto* name = "MorphObject";

    auto heap_type = reinterpret_cast<PyHeapTypeObject*>(
        PyType_Type.tp_alloc(&PyType_Type, 0));
    
    if (!heap_type)
        throw std::runtime_error("Failed to allocate new base class type.");;

    heap_type->ht_name = PyUnicode_FromString(name);
    heap_type->ht_qualname = heap_type->ht_name;
    Py_XINCREF(heap_type->ht_name);
    
    auto type = &heap_type->ht_type;
    type->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HEAPTYPE;
    type->tp_name = name;
    type->tp_base = &PyBaseObject_Type;
    Py_XINCREF(&PyBaseObject_Type);

    type->tp_basicsize = sizeof(Instance);

    type->tp_new = new_instance;
    type->tp_init = init_instance;
    type->tp_dealloc = dealloc_instance;

    if (PyType_Ready(type) < 0)
        throw std::runtime_error("Failed to create new base class type.");
    
    return reinterpret_cast<PyTypeObject*>(type);
}


type_object make_new_type(const char* name, Object nmspace)
{  
    auto base_class = internals().base_class();
    auto abc_meta = internals().abc_meta().type_ptr();

    auto name_obj = PyUnicode_FromString(name);
    auto bases = PyTuple_Pack(1, base_class.ptr());
    
    auto args = PyTuple_Pack(3, name_obj, bases, nmspace.ptr());

    auto type = reinterpret_cast<PyTypeObject*>(
        abc_meta->tp_new(abc_meta, args, nullptr));
    
    if (!type)
    {
        throw std::runtime_error("Failed to allocate new type.");
    }

    type->tp_dealloc = dealloc_instance;

    if (PyType_Ready(type))
    {
        throw std::runtime_error("Failed to create new type.");
    }
    
    return type;
}


Handle abstract_method_new() 
{
    auto type = internals().abstract_method_type();

    auto method = PyObject_GC_New(
        PyObject,
        type.type_ptr());
    
    if (!method)
    {
        throw std::runtime_error("Failed to allocate abstract method instance.");
    }

    return method;
}


PyObject* get_true(PyObject*, void*)
{
    Py_XINCREF(Py_True);
    return Py_True;
}


PyObject* abstract_method_call(PyObject*, PyObject*, PyObject*)
{
    PyErr_Format(PyExc_TypeError, "Trying to call pure abstract method.");
    return nullptr;
}

type_object make_abstract_method_type()
{
    auto heap_type = reinterpret_cast<PyHeapTypeObject*>(PyType_Type.tp_alloc(&PyType_Type, 0));

    if (!heap_type)
    {
        throw std::runtime_error("Failed to allocate AbstractMethod type");
    }

    constexpr const char* name = "AbstractMethod";

    heap_type->ht_name = PyUnicode_FromString(name);
    heap_type->ht_qualname = heap_type->ht_name;
    Py_XINCREF(heap_type->ht_name);

    auto type = &heap_type->ht_type;
    type->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE;
    type->tp_name = name;
    type->tp_base = &PyBaseObject_Type;
    Py_XINCREF(&PyBaseObject_Type);

    static PyGetSetDef getset[] = {
        {"__isabstractmethod__", get_true, nullptr, nullptr},
        {0}
    };

    type->tp_getset = getset;
    type->tp_call = abstract_method_call;

    if (PyType_Ready(type) < 0)
    {
        throw std::runtime_error("Failed to create new base class type.");
    }

    return reinterpret_cast<PyTypeObject*>(type);
}

Object make_abstract_method_instance(const char* name)
{
    auto method = abstract_method_new();

    return method.ptr();
}


} // namespace detail
} // namespace py
