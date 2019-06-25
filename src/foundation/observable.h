#pragma once

#include "foundation/immutable/map.h"

#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace foundation
{

template <typename... Args>
class Observable : public std::enable_shared_from_this<Observable<Args...>>
{
  public:
    using OnUpdateFn      = std::function<void(Args...)>;
    using SubscriptionKey = std::uint64_t;
    using Subscribers     = foundation::immutable::Map<SubscriptionKey, OnUpdateFn>;

    class Disposable
    {
      public:
        using DisposeFn = std::function<void ()>;

        explicit Disposable(DisposeFn on_dispose)
          : m_on_dispose(on_dispose)
        {}

        void dispose()
        {
            if (m_on_dispose)
            {
                m_on_dispose();
            }
        }

      private:
        DisposeFn m_on_dispose;
    };

    Observable()
      : m_next_subscription_key(0u)
    {}

    Disposable subscribe(OnUpdateFn on_update) noexcept;

  protected:
    void notify(Args... args) const;
    void unsubscribe(SubscriptionKey key);

  private:
    SubscriptionKey     m_next_subscription_key;
    Subscribers         m_subscribers;
    mutable std::mutex  m_mutex;
};

template <typename... Args>
typename Observable<Args...>::Disposable Observable<Args...>::subscribe(OnUpdateFn on_update) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto key = m_next_subscription_key++;
    m_subscribers = m_subscribers.set(key, std::move(on_update));

    return Disposable(
        [thisWeakPtr = this->weak_from_this(), key]()
        {
            if (const auto thisPtr = thisWeakPtr.lock())
            {
                thisPtr->unsubscribe(key);
            }
        });
}

template <typename... Args>
void Observable<Args...>::notify(Args... args) const
{
    Subscribers subscribers;

    m_mutex.lock();
    subscribers = m_subscribers;
    m_mutex.unlock();

    for (const auto& cb : subscribers)
    {
        try
        {
            cb.second(args...);
        }
        catch(const std::exception&)
        {
            // TODO: proper error handling.
        }
    }
}

template <typename... Args>
void Observable<Args...>::unsubscribe(SubscriptionKey key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subscribers = m_subscribers.erase(key);
}

} // namespace foundation
