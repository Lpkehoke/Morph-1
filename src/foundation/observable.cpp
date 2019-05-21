#include "observable.h"

#include <tbb/spin_mutex.h>

#include <exception>
#include <utility>
#include <list>
#include <memory>

namespace foundation
{
    template <typename T>
    struct observable<T>::impl_observable_t
    {
        std::list<on_update_fn> m_subscribers;
        tbb::spin_mutex         m_mutex;
    };

    template <typename T>
    observable<T>::observable()
        : impl_observable(new impl_observable_t())
    {}

    template <typename T>
    observable<T>::~observable()
    {
        delete impl_observable;
    }

    template <typename T>
    auto observable<T>::subscribe(on_update_fn on_update) noexcept
    {
        tbb::spin_mutex::scoped_lock lock(impl_observable->m_mutex);
        impl_observable->m_subscribers.emplace_back(std::move(on_update));

        auto current_fun_it = --impl_observable->m_subscribers.cend();
        auto weak_this = this->weak_from_this();

        bool function_is_alive = true;

        return [=]() mutable
        {
            if (function_is_alive && !weak_this.expired())
            {
                function_is_alive = false;

                impl_observable->m_mutex.lock();
                impl_observable->m_subscribers.erase(current_fun_it);
                impl_observable->m_mutex.unlock();
            }
        };
    }

} // namespace foundation
