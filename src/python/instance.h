#pragma once

#include "python/pythonapi.h"

#include <typeinfo>
#include <unordered_map>
#include <typeindex>

namespace py
{
namespace detail
{

struct instance
{
    using held_t = std::unordered_map<std::type_index, void*>;

    PyObject_HEAD

    held_t      m_held;
    PyObject*   m_dict;
};


PyObject* new_instance(PyTypeObject* subtype, PyObject* args, PyObject*)
{
    auto inst = reinterpret_cast<instance*>(subtype->tp_alloc(subtype, 0));
    new (&inst->m_held) instance::held_t();

    return reinterpret_cast<PyObject*>(inst);
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


PyObject* base_class_get_dict(PyObject* self, void* )
{
    auto& dict = *_PyObject_GetDictPtr(self);
    if (!dict)
        dict = PyDict_New();
    Py_XINCREF(dict);
    return dict;
}


int base_class_set_dict(PyObject* self, PyObject* new_dict, void*)
{
    if (!PyDict_Check(new_dict)) {
        PyErr_Format(PyExc_TypeError, "__dict__ must be of dict type");
        return -1;
    }

    PyObject *&dict = *_PyObject_GetDictPtr(self);
    Py_INCREF(new_dict);
    Py_CLEAR(dict);
    dict = new_dict;
    return 0;
}


type_object make_new_base_class()
{
    constexpr auto* name = "MorphObject";

    static PyGetSetDef getset[] = {
        {const_cast<char*>("__dict__"), base_class_get_dict, base_class_set_dict, nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr}
    };

    auto heap_type = reinterpret_cast<PyHeapTypeObject*>(
        PyType_Type.tp_alloc(&PyType_Type, 0));
    
    if (!heap_type)
        throw std::runtime_error("Failed to allocate new base class type.");;

    heap_type->ht_name = PyUnicode_FromString(name);
    heap_type->ht_qualname = heap_type->ht_name;
    Py_XINCREF(heap_type->ht_name);
    
    auto type = &heap_type->ht_type;
    type->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
                    | Py_TPFLAGS_HEAPTYPE | Py_TPFLAGS_HAVE_GC;
    type->tp_name = name;
    type->tp_base = &PyBaseObject_Type;
    Py_XINCREF(&PyBaseObject_Type);

    type->tp_dictoffset = offsetof(instance, m_dict);
    type->tp_getset = getset;

    type->tp_basicsize = sizeof(instance);

    type->tp_new = new_instance;
    type->tp_init = init_instance;
    //type->tp_dealloc = todo;

    if (PyType_Ready(type) < 0)
        throw std::runtime_error("Failed to create new base class type.");
    
    return reinterpret_cast<PyTypeObject*>(type);
}


type_object make_new_type(const char* name, type_object base_class)
{  
    PyType_Slot spec_slots[] = {
        {Py_tp_new, reinterpret_cast<void*>(&new_instance)},
        {Py_tp_init, reinterpret_cast<void*>(&init_instance)},
        {Py_tp_base, reinterpret_cast<void*>(base_class.inc_ref().ptr())},
        {0}
    };

    PyType_Spec spec;

    spec.basicsize = sizeof(instance);
    spec.name = name;
    spec.itemsize = 0;
    spec.flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE | Py_TPFLAGS_BASETYPE;
    spec.slots = spec_slots;

    auto heap_type = reinterpret_cast<PyHeapTypeObject*>(
        PyType_FromSpec(&spec));
    
    if (!heap_type)
        throw std::runtime_error("Failed to create new type.");
    
    return reinterpret_cast<PyTypeObject*>(heap_type);
}


} // namespace detail
} // namespace py
