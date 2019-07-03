#include "foundation/immutable/map.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

using namespace foundation::immutable;
using namespace testing;

template <typename T>
struct CollisionHash
{
    std::size_t operator()(const T&) const
    {
        return 0;
    }
};

//
// Immutable detail tests.
//

TEST(immutable_detail, make_array)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array<detail::HeapMemoryPolicy, int>(a, 7);

    for (auto i = 0; i < 7; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(immutable_detail, make_array_insert_begin)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_insert<detail::HeapMemoryPolicy, int>(a, 7, 0, 666);

    ASSERT_EQ(666, b[0]);
    for (auto i = 1; i < 8; ++i)
    {
        ASSERT_EQ(a[i - 1], b[i]);
    }
}

TEST(immutable_detail, make_array_insert_middle)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_insert<detail::HeapMemoryPolicy, int>(a, 7, 3, 666);

    for (auto i = 0; i < 3; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    ASSERT_EQ(666, b[3]);

    for (auto i = 4; i < 8; ++i)
    {
        ASSERT_EQ(a[i - 1], b[i]);
    }
}

TEST(immutable_detail, make_array_insert_end)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_insert<detail::HeapMemoryPolicy, int>(a, 7, 7, 666);

    for (auto i = 0; i < 7; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    ASSERT_EQ(666, b[7]);
}

TEST(immutable_detail, make_array_replace_begin)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_replace<detail::HeapMemoryPolicy, int>(a, 7, 0, 666);

    ASSERT_EQ(666, b[0]);
    for (auto i = 1; i < 7; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(immutable_detail, make_array_replace_middle)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_replace<detail::HeapMemoryPolicy, int>(a, 7, 3, 666);

    for (auto i = 0; i < 7; ++i)
    {
        if (i == 3)
        {
            ASSERT_EQ(666, b[i]);
            continue;
        }

        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(immutable_detail, make_array_replace_end)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_replace<detail::HeapMemoryPolicy, int>(a, 7, 6, 666);

    for (auto i = 0; i < 6; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    ASSERT_EQ(666, b[6]);
}

TEST(immutable_detail, make_array_erase_begin)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_erase<detail::HeapMemoryPolicy, int>(a, 7, 0);

    for (auto i = 0; i < 6; ++i)
    {
        ASSERT_EQ(a[i + 1], b[i]);
    }
}

TEST(immutable_detail, make_array_erase_middle)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_erase<detail::HeapMemoryPolicy, int>(a, 7, 3);

    for (auto i = 0; i < 3; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    for (auto i = 3; i < 6; ++i)
    {
        ASSERT_EQ(a[i + 1], b[i]);
    }
}

TEST(immutable_detail, make_array_erase_end)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = detail::make_array_erase<detail::HeapMemoryPolicy, int>(a, 7, 6);

    for (auto i = 0; i < 6; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }
}

//
// Immutable map tests.
//

TEST(immutable_map, basic_set)
{
    using MapType = Map<int, int>;
    MapType m;

    std::function<MapType(int)> set = [m](int value)
    {
        return m.set(value, value);
    };

    for (int i = 0; i < 64 * 64; ++i)
    {
        auto new_map = set(i);

        ASSERT_EQ(new_map.size(), static_cast<std::size_t>(i + 1));

        for (int j = 0; j < i; ++j)
        {
            ASSERT_TRUE(new_map.get(j) != nullptr);
            ASSERT_EQ(*new_map.get(j), j);
        }

        set = [new_map](int value)
        {
            return new_map.set(value, value);
        };
    }
}

TEST(immutable_map, collision_set)
{
    using MapType = Map<int, int, CollisionHash<int>>;
    MapType m;

    for (int i = 0; i < 64; ++i)
    {
        m = m.set(i, i);

        ASSERT_EQ(m.size(), static_cast<std::size_t>(i + 1));

        for (int j = 0; j < i; ++j)
        {
            ASSERT_TRUE(m.get(j) != nullptr);
            ASSERT_EQ(*m.get(j), j);
        }
    }
}

TEST(immutable_map, basic_erase)
{
    using MapType = Map<int, int>;
    MapType m;

    for (int i = 0; i < 64 * 16; ++i)
    {
        m = m.set(i, i);
    }

    ASSERT_EQ(m.size(), static_cast<std::size_t>(64 * 16));

    for (int i = 64 * 16 - 1; i >= 0; --i)
    {
        m = m.erase(i);

        ASSERT_EQ(m.size(), static_cast<std::size_t>(i));

        for (int j = 0; j < i; ++j)
        {
            ASSERT_TRUE(m.get(j) != nullptr);
            ASSERT_EQ(*m.get(j), j);
        }

        for (int j = i; j < 64 * 16; ++j)
        {
            ASSERT_TRUE(m.get(j) == nullptr);
        }
    }

    ASSERT_EQ(m.size(), 0u);
}

TEST(immutable_map, collision_erase)
{
    using MapType = Map<int, int, CollisionHash<int>>;
    MapType m;

    for (int i = 0; i < 64; ++i)
    {
        m = m.set(i, i);
    }

    for (int i = 64 - 1; i >= 0; --i)
    {
        m = m.erase(i);

        ASSERT_EQ(m.size(), static_cast<std::size_t>(i));

        for (int j = 0; j < i; ++j)
        {
            ASSERT_TRUE(m.get(j) != nullptr);
            ASSERT_EQ(*m.get(j), j);
        }

        for (int j = i; j < 64; ++j)
        {
            ASSERT_TRUE(m.get(j) == nullptr);
        }
    }

    ASSERT_EQ(m.size(), 0u);
}

//
// Immutable map iterator tests.
//

using map_t = Map<int, int>;

TEST(immutable_map, iterator_common_function)
{
    map_t a;

    ASSERT_TRUE(a.begin() == a.end());

    a = a.set(0, 10);

    ASSERT_TRUE(a.begin() == a.begin());
    ASSERT_TRUE(a.begin() != a.end());
    ASSERT_TRUE((*(a.begin())).first == 0);
    ASSERT_TRUE((*(a.begin())).second == 10);
    ASSERT_TRUE((++a.begin()) == a.end());
}

TEST(immutable_map, ranged_for_iteration)
{
    constexpr int num_el = 64 * 64;
    map_t m;

    for (int i = 0; i < num_el; ++i)
    {
        m = m.set(i, i);
    }

    int counter = 0;

    for ([[maybe_unused]] auto p : m)
    {
        ++counter;
    }

    ASSERT_EQ(counter, num_el);
}

TEST(immutable_map, ranged_for_iteration_empty_map)
{
    map_t m;

    int counter = 0;
    for ([[maybe_unused]] auto p : m)
    {
        ++counter;
    }

    ASSERT_EQ(counter, 0);
}

TEST(immutable_map, iterator_collision)
{
    constexpr int num_el = 64;
    using map_t = Map<int, int, CollisionHash<int>>;
    map_t m;

    for (int i = 0; i < num_el; ++i)
    {
        m = m.set(i, i);
    }

    int counter = 0;

    for ([[maybe_unused]] auto p : m)
    {
        ++counter;
    }

    ASSERT_EQ(counter, num_el);
}

//
// AnyTypeMap tests.
//

TEST(any_type_map, equal_value)
{
    AnyTypeMap<int> a;

    for (int i = 0; i < 64; ++i)
    {
        a = a.set(i, i);
    }

    ASSERT_EQ(static_cast<int>(a.size()), 64);
}

TEST(any_type_map, simple_usage)
{
    AnyTypeMap<int> a;

    a = a.set(0, 0);
    a = a.set(1, true);
    a = a.set(2, std::string("string"));
    a = a.set(3, "string");

    ASSERT_EQ(static_cast<int>(a.size()), 4);
}

TEST(any_type_map, get_data)
{
    AnyTypeMap<int> a;

    a = a.set(0, 0);
    a = a.set(1, true);

    ASSERT_EQ(*a[0]->cast<int>(), 0);
    ASSERT_EQ(*a[1]->cast<bool>(), true);
}

TEST(any_type_map, erase)
{
    AnyTypeMap<int> a;

    a = a.set(0, 0);
    ASSERT_EQ(static_cast<int>(a.size()), 1);
    a = a.erase(0);
    ASSERT_EQ(static_cast<int>(a.size()), 0);
}

TEST(any_type_map, getattr)
{
    AnyTypeMap<int> a;
    a = a.set(0, 0);

    ASSERT_EQ(*(a.getattr<int>(0)), 0);
    ASSERT_EQ((a.getattr<bool>(0)), nullptr);
}

TEST(any_type_map, bad_cast)
{
    AnyTypeMap<int> a;

    a = a.set(0, 0);

    try
    {
        auto el = a[0]->cast<bool>();
        ASSERT_TRUE(false);
    }
    catch (const std::bad_cast& ex)
    {
        ASSERT_EQ(static_cast<int>(a.size()), 1);
    }
}
