#include "python/internals.h"

namespace py
{
namespace detail
{

internals_t& internals()
{
    static internals_t::ptr_t ptr;

    if (!ptr)
    {
        ptr.reset(new internals_t {});
    }

    return *ptr;
}

void register_instance(const void* value, instance* inst)
{
    internals().m_registered_instances.emplace(
        static_cast<const void*>(value),
        inst);
}

void unregister_instance(instance* inst)
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

python_type_info* get_python_type_info(handle py_type)
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
