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


struct type_info
{
    type_object             m_py_type;
    const std::type_info&   m_cpp_type;
};


struct internals_t
{
    using ptr_t = std::unique_ptr<internals_t>;
    using type_map_t = std::unordered_map<std::type_index, type_info>;

    template <typename T>
    void register_type(type_object py_type_obj)
    {
        auto& tinfo = typeid(T);
        m_registered_types.emplace(tinfo, type_info {py_type_obj, tinfo});
    }

    template <typename T>
    type_info& type_info_for_()
    {
        auto itr = m_registered_types.find(typeid(T));
        if (itr == m_registered_types.end())
        {
            throw std::runtime_error("Type is not registered.");
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
