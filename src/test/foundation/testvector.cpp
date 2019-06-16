#include "foundation/vecor.h"

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
