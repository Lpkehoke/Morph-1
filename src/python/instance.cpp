#include "python/instance.h"

#include "python/pythonapi.h"
#include "python/internals.h"

#include <typeinfo>
#include <unordered_map>
#include <typeindex>

namespace py
{
namespace detail
{

PyObject* make_new_instance(PyTypeObject* subtype)
{
    auto inst = reinterpret_cast<instance*>(subtype->tp_alloc(subtype, 0));
    inst->m_value_and_holder = nullptr;

    return reinterpret_cast<PyObject*>(inst);
}


PyObject* new_instance(PyTypeObject* subtype, PyObject* args, PyObject*)
{
    return make_new_instance(subtype);
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


int traverse_instance(PyObject* self, visitproc visit, void* arg)
{
    auto inst = reinterpret_cast<instance*>(self);
    Py_VISIT(inst->m_dict);
    return 0;
}


int clear_instance(PyObject* self)
{
    auto inst = reinterpret_cast<instance*>(self);
    Py_CLEAR(inst->m_dict);
    return 0;
}


void dealloc_instance(PyObject* self)
{
    auto inst = reinterpret_cast<instance*>(self);

    if (inst->m_value_and_holder)
    {
        internals().unregister_instance(inst->m_value_and_holder->m_held.get());
        delete inst->m_value_and_holder;
        inst->m_value_and_holder = nullptr;
    }

    auto type = Py_TYPE(self);
    type->tp_free(self);
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

    type->tp_basicsize = sizeof(instance);

    type->tp_new = new_instance;
    type->tp_init = init_instance;
    type->tp_dealloc = dealloc_instance;

    if (PyType_Ready(type) < 0)
        throw std::runtime_error("Failed to create new base class type.");
    
    return reinterpret_cast<PyTypeObject*>(type);
}


type_object make_new_type(const char* name, type_object base_class)
{  
    auto abc_meta = internals().abc_meta().type_ptr();

    auto name_obj = PyUnicode_FromString(name);
    auto bases = PyTuple_Pack(1, base_class.ptr());
    auto dict = PyDict_New();
    PyDict_SetItemString(dict, "__module__", PyUnicode_FromString("KEK"));
    
    auto args = PyTuple_Pack(3, name_obj, bases, dict);

    auto heap_type = reinterpret_cast<PyHeapTypeObject*>(
        abc_meta->tp_new(abc_meta, args, nullptr));
    
    if (!heap_type)
    {
        throw std::runtime_error("Failed to allocate new type.");
    }

    heap_type->ht_type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE
        | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC;

    auto type = reinterpret_cast<PyTypeObject*>(heap_type);

    if (PyType_Ready(type))
    {
        throw std::runtime_error("Failed to create new type.");
    }
    
    return type;
}


} // namespace detail
} // namespace py
