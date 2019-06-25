#include "foundation/vector.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace foundation;

TEST(vector2, add)
{
    Vector2f a(0.0f, 1.0f);
    Vector2f b(2.0f, 3.0f);

    auto res = Vector2f {2.0f, 4.0f};

    ASSERT_EQ(res, a + b);
}

TEST(vector2, sub)
{
    Vector2f a(0.0f, 1.0f);
    Vector2f b(2.0f, 3.0f);

    auto res = Vector2f {-2.0f, -2.0f};

    ASSERT_EQ(res, a - b);
}

TEST(vector2, scalar_mult)
{
    Vector2f a(2.0f, 1.0f);
    float factor = 5.0f;

    auto res = Vector2f {10.0f, 5.0f};

    ASSERT_EQ(res, a * factor);
}

TEST(vector2, unary_minus)
{
    Vector2f a(1.0f, -1.0f);
    Vector2f b(-1.0f, 1.0f);

    ASSERT_EQ(a, -b);
}

TEST(vector2, equals)
{
    Vector2f a(1.0f, 1.0f);
    Vector2f b(1.0f, 1.0f);
    Vector2f c(-1.0f, -1.0f);

    ASSERT_TRUE(a == b);
    ASSERT_TRUE(!(a == c));
    ASSERT_TRUE(a != c);
    ASSERT_TRUE(!(a != b));
}

TEST(vector2, dot_product)
{
    Vector2f a(1.0f, 2.0f);
    Vector2f b(3.0f, 4.0f);

    auto dp = dot_product(a, b);

    ASSERT_EQ(11.0f, dp);
}

//
// Vector3 tests
//

TEST(vector3, add)
{
    Vector3f a(0.0f, 1.0f, 4.0f);
    Vector3f b(2.0f, 3.0f, 5.0f);

    auto res = Vector3f {2.0f, 4.0f, 9.0f};

    ASSERT_EQ(res, a + b);
}

TEST(vector3, sub)
{
    Vector3f a(0.0f, 1.0f, 4.0f);
    Vector3f b(2.0f, 3.0f, 5.0f);

    auto res = Vector3f {-2.0f, -2.0f, -1.0f};

    ASSERT_EQ(res, a - b);
}

TEST(vector3, scalar_mult)
{
    Vector3f a(2.0f, 1.0f, 4.0f);
    float factor = 5.0f;

    auto res = Vector3f {10.0f, 5.0f, 20.0f};

    ASSERT_EQ(res, a * factor);
}

TEST(vector3, unary_minus)
{
    Vector3f a(1.0f, -1.0f, 0);
    float b = -1.0f;

    auto res = -a;
    ASSERT_EQ(res, a * b);
}

TEST(vector3, equals)
{
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(1.0f, 2.0f, 3.0f);
    Vector3f c(-1.0f, 2.0f, 3.0f);

    ASSERT_TRUE(a == b);
    ASSERT_TRUE(!(a == c));
    ASSERT_TRUE(a != c);
    ASSERT_TRUE(!(a != b));
}

TEST(vector3, dot_product)
{
    Vector3f a(1.0f, 2.0f, 7.0f);
    Vector3f b(3.0f, -4.0f, 6.0f);

    auto dp = dot_product(a, b);

    ASSERT_EQ(37.0f, dp);
}

TEST(vector3, cross_product)
{
    Vector3f a(1.0f, 2.0f, 7.0f);
    Vector3f b(3.0f, -4.0f, 6.0f);

    auto cp = cross_product(a, b);
    Vector3f c(40.0f, 15.0f, -10.0f);

    ASSERT_EQ(c, cp);
}
