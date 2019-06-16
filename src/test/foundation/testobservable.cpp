#include "foundation/observable.h"

#include <thread>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tbb/spin_mutex.h>

using namespace foundation;
using namespace testing;
using namespace std;

using State = vector<int>;

class AObs : public Observable<State>
{
  public:
    void add(int a)
    {
        m_state.push_back(a);
        notify(m_state);
    }

  private:
    State m_state;
};

TEST(observable_test, observable_simple_working)
{
    int b = 0;
    auto a = make_shared<AObs>();
    a->subscribe([&b](auto)
    {
        b++;
    });

    a->add(0);
    std::this_thread::sleep_for(0.1s);
    ASSERT_EQ(b, 1);
}

TEST(observable_test, observable_change_state)
{
    auto a = make_shared<AObs>();
    State a_state;

    a->subscribe([&a_state](const auto& state)
    {
        a_state = state;
    });

    for (int i = 0; i < 10; ++i)
    {
        a->add(i);
    }

    a->subscribe([&a_state](const auto& state)
    {
        for (size_t i = 0; i < state.size(); ++i)
        {
            ASSERT_EQ(state[i], a_state[i]);
        }
    });

    for (int i = 0; i < 10; ++i)
    {
        a->add(i);
    }

    std::this_thread::sleep_for(0.1s);
}

TEST(observable_test, observable_double_unsubscribe)
{
    auto a = make_shared<AObs>();
    auto un_1 = a->subscribe([](const auto& state){});
    un_1.dispose();
    un_1.dispose();
}

TEST(observable_test, unsubscribe_after_delete_observable)
{
    auto a = make_shared<AObs>();
    auto un_1 = a->subscribe([](auto state){});
    a.reset();
    un_1.dispose();
    un_1.dispose();
}
