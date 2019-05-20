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
    box()
      : m_pointee_type(nullptr)
    {}

    template <typename T>
    box(T&& value)
      : m_pointee_type(&detail::type_info_for_<T>)
      , m_held(new T(std::forward<T>))
    {}

    template <typename T>
    void store(T&& value)
    {
        store<T>(std::forward<T>(value), std::default_delete<T>{});
    }

    template <typename T, typename Deleter>
    void store(T&& value, Deleter&& deleter)
    {
        m_pointee_type(&detail::type_info_for_<T>);
        m_held = std::make_shared<T>(
            std::forward<T>(value),
            std::forward<Deleter>(deleter));
    }

    explicit operator bool() const
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
