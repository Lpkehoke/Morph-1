#pragma once

#include "python/internals.h"

#include <cstddef>
#include <set>
#include <sstream>
#include <string>

namespace test
{

class internals_inspector
{
  public:
    std::size_t instances_count() const
    {
        std::set<py::detail::instance*> unique_instances;

        const auto& instances_collection = py::detail::internals().m_registered_instances;

        for (const auto& pair : instances_collection)
        {
            unique_instances.insert(pair.second);
        }
        
        return unique_instances.size();
    }

    std::string dump_instances() const
    {
        std::ostringstream oss;

        const auto& instances_collection = py::detail::internals().m_registered_instances;

        // Iterate over unique keys.
        for (auto inst_pair_itr = instances_collection.begin();
             inst_pair_itr != instances_collection.end();
             inst_pair_itr = instances_collection.equal_range(inst_pair_itr->first).second)
        {
            // TODO: make repr() a free function?
            auto str = inst_pair_itr->second
                ? py::handle(reinterpret_cast<PyObject*>(inst_pair_itr->second)).repr()
                : "<Null handle>";
            
            oss << str << " (cpp types: ";

            // Print all known cpp types for this instance.
            for (const auto& inst_data : inst_pair_itr->second->m_held)
            {
                oss << inst_data.first.name() << "; ";
            }
            oss << " )" << std::endl;
        }

        return oss.str();
    }

    std::size_t types_count() const
    {
        return py::detail::internals().m_registered_types_cpp.size();
    }
};

} // namespace test
