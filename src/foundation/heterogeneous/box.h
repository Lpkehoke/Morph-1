#pragma once

#include "foundation/heterogeneous/typeinfo.h"

#include <memory>
#include <typeinfo>
#include <utility>


namespace foundation
{
namespace heterogeneous
{

class Box
{
  public:
    Box()
      : m_pointee_type(nullptr)
    {}

    Box(const Box& other)
      : m_pointee_type(other.m_pointee_type)
      , m_held(other.m_held)
    {}

    Box(Box&& other)
    {
        m_pointee_type = std::exchange(other.m_pointee_type, nullptr);
        std::swap(m_held, other.m_held);
    }

    template <typename T>
    Box(T* src, bool take_ownership)
    {
        initialize(src, take_ownership);
    }

    Box& operator=(Box&& other)
    {
        if (other.m_held != m_held)
        {
            std::swap(m_pointee_type, other.m_pointee_type);
            std::swap(m_held, other.m_held);
        }

        return *this;
    }

    template <typename T>
    void store(T* src, bool take_ownership)
    {
        initialize(src, take_ownership);
    }

    template <typename T>
    std::shared_ptr<T> load() const
    {
        return m_held
            ? std::reinterpret_pointer_cast<T>(m_held)
            : std::shared_ptr<T>();
    }

    void* raw_ptr()
    {
        return m_held.get();
    }

    template <typename T>
    bool is() const
    {
        return m_pointee_type
            ? m_pointee_type->m_tinfo->hash_code() == typeid(T).hash_code()
            : false;
    }

    explicit operator bool() const
    {
        return static_cast<bool>(m_held);
    }

  private:
    using Holder = std::shared_ptr<void>;

    const TypeInfo*     m_pointee_type;
    Holder              m_held;
};

} // namespace heterogeneous
} // namespace foundation
