#include "foundation/sharedany.h"
#include "foundation/immutable/anytypemap.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

using namespace foundation::immutable;
using namespace foundation;
using namespace testing;

//
// Shared any.
//

TEST(sharedany, basic_set)
{
    SharedAny a(0);

    ASSERT_TRUE(a.template is<int>());
    ASSERT_EQ(*a.template cast<int>(), 0);

    ASSERT_FALSE(a.template is<long>());
    ASSERT_THROW(a.template cast<long>(), std::bad_cast);
}

TEST(sharedany, shadow_equal_type)
{
    using int1 = int;
    using int2 = int;

    SharedAny a(static_cast<int1>(0));
    ASSERT_TRUE(a.template is<int2>());
}

//
// AnyTypeMap.
//

TEST(any_type_map, basic_set_equal_value)
{
    AnyTypeMap<int> a;

    for (int i = 0; i < 64; ++i)
    {
        a = a.set(i, i);
    }

    ASSERT_EQ(static_cast<int>(a.size()), 64);
}

TEST(any_type_map, basic_set)
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

TEST(any_type_map, basic_erase)
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
    ASSERT_THROW(a[0]->cast<bool>(), std::bad_cast);
}
