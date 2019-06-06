#pragma once

#include "python/instance.h"
#include "python/pythonapi.h"

#include "foundation/heterogeneous/typeinfo.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace py
{
namespace detail
{

/**
 *  Holds information neccessary for casting/loading
 *  python values from instances.
 */
struct python_type_info
{
    using type_info_t = foundation::heterogeneous::type_info;
    using conversions_set_t = std::unordered_set<std::type_index>;

    const type_info_t*  m_cpp_tinfo;
    conversions_set_t   m_conversions;
    type_object         m_py_type;
};

/**
 *  Internal structure keeping track of all created types and instances.
 */
struct internals_t
{
    using ptr_t = std::unique_ptr<internals_t>;
    using type_map_t = std::unordered_map<std::type_index, python_type_info*>;
    using py_type_map_t = std::unordered_map<PyObject*, python_type_info*>;
    using inst_map_t = std::unordered_multimap<const void*, instance*>;

    type_object& base_class()
    {
        if (!m_base_class)
        {
            m_base_class = make_new_base_class();
        }

        return m_base_class;
    }

    type_object abc_meta()
    {
        if (!m_abc_module)
        {
            handle m = PyImport_ImportModule("abc");
            if (!m)
            {
                throw std::runtime_error("Failed to load abc module.");
            }

            m_abc_module = m.ptr();
        }

        return m_abc_module.getattr("ABCMeta").ptr();
    }

    type_object abstract_method_type()
    {
        if (!m_abstract_method_type)
        {
            m_abstract_method_type = make_abstract_method_type();
        }

        if (!m_abstract_method_type)
        {
            throw std::runtime_error("Failed to create AbstractMethod type.");
        }

        return m_abstract_method_type;
    }

    type_map_t      m_registered_types_cpp;
    py_type_map_t   m_registered_types_py;
    inst_map_t      m_registered_instances;
    type_object     m_base_class;
    type_object     m_abstract_method_type;
    object          m_abc_module;
};

extern internals_t& internals();

template <typename T>
void register_python_type(
    type_object                         py_type_obj,
    python_type_info::conversions_set_t conversions)
{
    using namespace foundation::heterogeneous;

    auto* py_tinfo = new python_type_info();
    py_tinfo->m_cpp_tinfo = &type_info_for_<T>;
    py_tinfo->m_py_type = py_type_obj;
    py_tinfo->m_conversions = std::move(conversions);

    internals().m_registered_types_cpp.emplace(typeid(T), py_tinfo);
    internals().m_registered_types_py.emplace(py_type_obj.ptr(), py_tinfo);
}

template <typename T>
python_type_info* get_python_type_info()
{
    auto& i = internals();
    auto itr = i.m_registered_types_cpp.find(typeid(T));

    if (itr == i.m_registered_types_cpp.end())
    {
        return nullptr;
    }

    return itr->second;
}

python_type_info* get_python_type_info(handle py_type);

void register_instance(const void* value, instance* inst);

template <typename T>
void register_instance(const T* value, instance* inst)
{
    register_instance(static_cast<const void*>(value), inst);
}

void unregister_instance(instance* inst);

template <typename T>
handle get_registered_instance(T* held)
{
    // There might be several instances holding one pointer.
    // e.g. a class with a single member returning it by reference.
    auto range = internals().m_registered_instances.equal_range(static_cast<void*>(held));
    for (auto inst_itr = range.first; inst_itr != range.second; ++inst_itr)
    {
        instance* inst = inst_itr->second;
        
        auto data_itr = inst->m_held.find(typeid(T));
        if (data_itr != inst->m_held.end())
        {
            return reinterpret_cast<PyObject*>(inst);
        }
    }

    return handle();
}

} // namespace detail
} // namespace py
