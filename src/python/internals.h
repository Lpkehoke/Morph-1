#pragma once

#include "python/instance.h"
#include "python/pythonapi.h"

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>


namespace py
{
namespace detail
{

struct internals_t
{
    using ptr_t = std::unique_ptr<internals_t>;
    using type_map_t = std::unordered_map<std::type_index, type_object>;
    using inst_map_t = std::unordered_map<const void*, handle>;

    template <typename T>
    void register_type(type_object py_type_obj)
    {
        m_registered_types.emplace(typeid(T), py_type_obj);
    }

    template <typename T>
    type_object type_info_for_()
    {
        auto itr = m_registered_types.find(typeid(T));
        if (itr == m_registered_types.end())
        {
            throw std::runtime_error("Type is not registered.");
        }

        return itr->second;
    }

    template <typename T>
    void register_instance(T* inst, handle py_obj)
    {
        m_registered_instances.emplace(static_cast<void*>(inst), py_obj);
    }

    template <typename T>
    handle object_for_(T* inst)
    {
        auto itr = m_registered_instances.find(static_cast<void*>(inst));
        if (itr == m_registered_instances.end())
        {
            return handle();
        }

        return itr->second;
    }

    type_object& base_class()
    {
        if (!m_base_class)
        {
            m_base_class = make_new_base_class();
        }

        return m_base_class;
    }

    type_map_t  m_registered_types;
    inst_map_t  m_registered_instances;
    type_object m_base_class;
};


static internals_t& internals()
{
    static internals_t::ptr_t ptr;

    if (!ptr)
    {
        ptr.reset(new internals_t {});
    }

    return *ptr;
}


} // namespace detail
} // namespace py
