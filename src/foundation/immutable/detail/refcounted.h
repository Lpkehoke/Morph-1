#pragma once

#include <atomic>
#include <cstdint>

namespace foundation
{
namespace immutable
{
namespace detail
{

struct refcounted
{
    using refcount_t = std::atomic<std::uint64_t>;

    refcounted()
        : m_ref_count(new refcount_t(1))
    {}

    refcounted(const refcounted& other)
        : m_ref_count(other.m_ref_count)
    {}

    ~refcounted()
    {
        if (is_unique())
        {
            delete m_ref_count;
        }
    }

    void inc()
    {
        m_ref_count->fetch_add(1, std::memory_order_relaxed);
    }

    bool dec()
    {
        return m_ref_count->fetch_sub(1, std::memory_order_release) == 1;
    }

    bool is_unique() const
    {
        return m_ref_count->load() == 1u;
    }

    refcount_t* m_ref_count;
};

} // namespace detail
} // namespace immutable
} // namespace foundation
