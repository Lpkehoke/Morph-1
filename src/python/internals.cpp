#include "python/internals.h"

namespace py
{
namespace detail
{

Internals& internals()
{
    static Internals::Ptr ptr;

    if (!ptr)
    {
        ptr.reset(new Internals {});
    }

    return *ptr;
}

void register_instance(const void* value, Instance* inst)
{
    internals().m_registered_instances.emplace(
        static_cast<const void*>(value),
        inst);
}

void unregister_instance(Instance* inst)
{
    for (const auto& inst_data_pair : inst->m_held)
    {    
        auto inst_data_ptr = inst_data_pair.second.get();
        auto range = internals().m_registered_instances.equal_range(inst_data_ptr);

        bool inst_data_found = false;
        for (auto itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second == inst)
            {
                internals().m_registered_instances.erase(itr);
                inst_data_found = true;
                break;
            }
        }

        assert(inst_data_found && "No matching instance found in registered instances collection.");
    }
}

PythonTypeInfo* get_python_type_info(Handle py_type)
{
    auto& i = internals();
    auto itr = i.m_registered_types_py.find(py_type.ptr());

    if (itr == i.m_registered_types_py.end())
    {
        return nullptr;
    }

    return itr->second;
}

} // namespace detail
} // namespace py
