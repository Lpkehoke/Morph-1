#include "foundation/sharedany.h"
#include "foundation/immutable/anytypemap.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <cstring>

using namespace foundation::immutable;
using namespace foundation;
using namespace testing;

//
// Shared any.
//

TEST(sharedany, basic_set)
{
    SharedAny a(0.0f);

    ASSERT_TRUE(a.template pure_is<float>());
    ASSERT_EQ(*a.template pure_cast<float>(), 0.0f);

    ASSERT_FALSE(a.template pure_is<long>());
    ASSERT_THROW(a.template pure_cast<long>(), std::bad_cast);
}

TEST(sharedany, not_pure_set_int)
{
    SharedAny a(0);

    ASSERT_TRUE(a.template is<long>());
    ASSERT_TRUE(a.template is<unsigned>());
    ASSERT_EQ(*a.template cast<long>(), 0l);
    ASSERT_EQ(*a.template cast<unsigned>(), 0u);
}

TEST(sharedany, not_pure_set_string)
{
    const char* str = static_cast<const char*>("aaa");
    std::string str2 = "aaa";

    SharedAny a(str);

    ASSERT_TRUE(a.template is<std::string>());
    ASSERT_EQ(*a.template cast<std::string>(), str2);

    SharedAny b(str2);

    ASSERT_TRUE(a.template is<char*>());
    ASSERT_EQ(std::strcmp(*a.template cast<char*>(), str), 0);
}


TEST(sharedany, shadow_equal_type)
{
    using int1 = int;
    using int2 = int;

    SharedAny a(static_cast<int1>(0));
    ASSERT_TRUE(a.template pure_is<int2>());
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

    ASSERT_EQ(*a[0]->pure_cast<int>(), 0);
    ASSERT_EQ(*a[1]->pure_cast<bool>(), true);
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
    ASSERT_THROW(a[0]->pure_cast<bool>(), std::bad_cast);
}
