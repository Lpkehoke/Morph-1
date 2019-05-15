#pragma once

#include "python/exceptions.h"
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

struct instance_info
{
    const std::type_info*   cpp_tinfo;
    handle                  py_obj;
};

struct internals_t
{
    using ptr_t = std::unique_ptr<internals_t>;
    using type_map_t = std::unordered_map<std::type_index, type_object>;
    using inst_map_t = std::unordered_multimap<const void*, instance_info>;

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
            throw std::runtime_error(std::string("Type ") + typeid(T).name() + " is not registered.");
        }

        return itr->second;
    }

    template <typename T>
    void register_instance(T* held, handle py_obj)
    {
        m_registered_instances.emplace(
            static_cast<void*>(held),
            instance_info {&typeid(T), py_obj});
    }

    template <typename T>
    void unregister_instance(T* held)
    {
        m_registered_instances.erase(static_cast<void*>(held));
    }

    template <typename T>
    handle object_for_(T* held)
    {
        // There might be several instances holding one pointer.
        // e.g. a class with a single member returning it by reference.
        auto range = m_registered_instances.equal_range(static_cast<void*>(held));
        for (auto itr = range.first; itr != range.second; ++itr)
        {
            const auto& inst_info = itr->second;
            if (inst_info.cpp_tinfo->hash_code() == typeid(T).hash_code())
            {
                return inst_info.py_obj;
            }
        }

        return handle();
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
