#pragma once

#include <atomic>
#include <cstdint>

namespace foundation
{
namespace immutable
{
namespace detail
{

struct Refcounted
{
    using Refcount = std::atomic<std::uint64_t>;

    Refcounted()
        : m_ref_count(new Refcount(1))
    {}

    Refcounted(const Refcounted& other)
        : m_ref_count(other.m_ref_count)
    {}

    ~Refcounted()
    {
        if (is_unique())
        {
            delete m_ref_count;
        }
    }

    void inc() noexcept
    {
        m_ref_count->fetch_add(1, std::memory_order_relaxed);
    }

    bool dec() noexcept
    {
        return m_ref_count->fetch_sub(1, std::memory_order_release) == 1;
    }

    bool is_unique() const noexcept
    {
        return m_ref_count->load() == 1u;
    }

    Refcount* m_ref_count;
};

} // namespace detail
} // namespace immutable
} // namespace foundation
