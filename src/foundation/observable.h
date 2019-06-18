#pragma once

#include <functional>
#include <utility>
#include <exception>
#include <memory>
#include <mutex>
#include <list>

#include "foundation/immutable/map.h"

namespace foundation
{

template <typename... Args>
class observable : public std::enable_shared_from_this<observable<Args...>>
{
  public:
    using on_update_fn = std::function<void(Args...)>;
    using store_t      = std::list<on_update_fn>;

    class disposable final
    {
        public:
            using weak_t = decltype(
                std::declval<observable<Args...>>().weak_from_this()
            );

            using it_t   = decltype(
                std::declval<store_t>().cend()
            );

            explicit disposable(
                const store_t& store,
                weak_t         weak,
                it_t           it,
                std::mutex*    mutex
            );

            void dispose() noexcept;
        private:
            store_t     m_subscribers;
            weak_t      m_weak_this;
            it_t        m_function_it;
            std::mutex* m_mutex;
            bool        m_function_is_alive;
    };

    disposable subscribe(on_update_fn&& on_update) noexcept;

    observable(const observable&)            = default;
    observable& operator=(const observable&) = default;
    observable(observable&&)                 = default;
    observable& operator=(observable&&)      = default;
    virtual ~observable()                    = default;

  protected:
    observable() = default;
    void notify(Args... args) const;

  private:
    store_t             m_subscribers;
    mutable std::mutex  m_mutex;

};

template <typename... Args>
typename observable<Args...>::disposable observable<Args...>::subscribe(on_update_fn&& on_update) noexcept
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_subscribers.emplace_back(std::move(on_update));

    return disposable(
        m_subscribers,
        this->weak_from_this(),
        --m_subscribers.cend(),
        &m_mutex
    );
}

template <typename... Args>
void observable<Args...>::notify(Args... args) const
{
    std::lock_guard<std::mutex> guard(m_mutex);

    for (const auto& cb : m_subscribers)
    {
        try
        {
            cb(args...);
        }
        catch(const std::exception& ex)
        {
            ex.what();
        }
        catch ( ... )
        {}
    }
}

template <typename... Args>
observable<Args...>::disposable::disposable(
    const store_t& store,
    weak_t         weak,
    it_t           it,
    std::mutex*    mutex
)
  : m_subscribers(store)
  , m_weak_this(weak)
  , m_function_it(it)
  , m_mutex(mutex)
  , m_function_is_alive(true)
{}

template <typename... Args>
void observable<Args...>::disposable::dispose() noexcept
{
    if (m_function_is_alive && !m_weak_this.expired())
    {
        m_function_is_alive = false;

        m_mutex->lock();
        m_subscribers.erase(m_function_it);
        m_mutex->unlock();
    }
}

} // namespace foundation
