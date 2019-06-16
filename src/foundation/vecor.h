#pragma once

#include <cstddef>
#include <utility>

namespace foundation
{

template <std::size_t Dimensions, typename Element>
class Vector;

template <typename Element>
class Vector<2, Element>
{
  public:
    static constexpr std::size_t dimensions = 2;

    Vector(Element x, Element y);

    Element& x();
    const Element& x() const;

    Element& y();
    const Element& y() const;

  private:
    Element m_data[2];
};

template <typename Element>
using Vector2 = Vector<2, Element>;

using Vector2f = Vector2<float>;

template <typename T>
Vector2<T> operator+(const Vector2<T>& lhs, const Vector2<T>& rhs);

template <typename T>
Vector2<T> operator-(const Vector2<T>& lhs, const Vector2<T>& rhs);

template <typename T>
Vector2<T> operator*(const Vector2<T>& lhs, const T& rhs);

template <typename T>
bool operator==(const Vector2<T>& lhs, const Vector2<T>& rhs);

//
//  Vector2 implementation.
//

template <typename T>
Vector<2, T>::Vector(T x, T y)
{
    m_data[0] = std::move(x);
    m_data[1] = std::move(y);
}

template <typename T>
const T& Vector<2, T>::x() const
{
    return m_data[0];
}

template <typename T>
const T& Vector<2, T>::y() const
{
    return m_data[1];
}

template <typename T>
Vector2<T> operator+(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
    return Vector2<T> {lhs.x() + rhs.x(), lhs.y() + rhs.y()};
}

template <typename T>
Vector2<T> operator-(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
    return Vector2<T> {lhs.x() - rhs.x(), lhs.y() - rhs.y()};
}

template <typename T>
Vector2<T> operator*(const Vector2<T>& lhs, const T& rhs)
{
    return Vector2<T> {lhs.x() * rhs, lhs.y() * rhs};
}

template <typename T>
bool operator==(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
    return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y());
}

} // namespace foundation
