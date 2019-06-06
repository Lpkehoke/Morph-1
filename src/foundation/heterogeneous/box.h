#pragma once

#include "foundation/heterogeneous/typeinfo.h"

#include <memory>
#include <typeinfo>
#include <utility>


namespace foundation
{
namespace heterogeneous
{

class box
{
  public:
    box() noexcept
      : m_pointee_type(nullptr)
    {}

    box(const box& other) noexcept
      : m_pointee_type(other.m_pointee_type)
      , m_held(other.m_held)
    {}

    box(box&& other)
    {
        m_pointee_type = std::exchange(other.m_pointee_type, nullptr);
        std::swap(m_held, other.m_held);
    }

    box& operator=(box&& other)
    {
        if (other.m_held != m_held)
        {
            std::swap(m_pointee_type, other.m_pointee_type);
            std::swap(m_held, other.m_held);
        }

        return *this;
    }

    template <typename T>
    std::shared_ptr<T> load() const noexcept
    {
        return std::reinterpret_pointer_cast<T>(m_held);
    }

    void* raw_ptr() noexcept
    {
        return m_held.get();
    }

    template <typename T>
    bool is_() const noexcept
    {
        return m_pointee_type
            ? m_pointee_type->m_tinfo->hash_code() == typeid(T).hash_code()
            : false;
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_held);
    }

  private:
    using holder_t = std::shared_ptr<void>;

    const type_info*    m_pointee_type;
    holder_t            m_held;
};

} // namespace heterogeneous
} // namespace foundation
