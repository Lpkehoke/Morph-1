#include "foundation/observable.h"

#include <thread>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tbb/spin_mutex.h>

using namespace foundation;
using namespace testing;
using namespace std;

using state_t = vector<int>;

class A_obs : public observable<const state_t*>
{
  public:
    A_obs() = default;
    ~A_obs() override = default;

    void add(int a)
    {
        m_state.push_back(a);
        notify(&m_state);
    }

  private:
    state_t m_state;
};

TEST(observable_test, observable_simple_working)
{
    int b = 0;
    auto a = make_shared<A_obs>();
    a->subscribe([&b](auto*)
    {
        b++;
    });

    a->add(0);
    std::this_thread::sleep_for(0.1s);
    ASSERT_EQ(b, 1);
}

TEST(observable_test, observable_change_state)
{
    auto a = make_shared<A_obs>();
    const state_t* a_state = nullptr;

    a->subscribe([&a_state](auto* state)
    {
        a_state = state;
    });

    for (int i = 0; i < 10; ++i)
    {
        a->add(i);
    }

    a->subscribe([&a_state](auto* state)
    {
        for (size_t i = 0; i < state->size(); ++i)
        {
            ASSERT_EQ((*state)[i], (*a_state)[i]);
        }
    });

    for (int i = 0; i < 10; ++i)
    {
        a->add(i);
    }

    std::this_thread::sleep_for(0.1s);
}

TEST(observable_test, observable_duble_unsubscribe)
{
    auto a = make_shared<A_obs>();
    auto un_1 = a->subscribe([](auto state){});
    un_1.dispose();
    un_1.dispose();
}

TEST(observable_test, unsubscribe_after_delete_observable)
{
    auto a = make_shared<A_obs>();
    auto un_1 = a->subscribe([](auto state){});
    a.reset();
    un_1.dispose();
    un_1.dispose();
}
