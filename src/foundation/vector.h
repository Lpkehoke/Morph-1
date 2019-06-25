#pragma once

#include <array>
#include <cstddef>
#include <utility>

namespace foundation
{

//
// Vector declaration.
//

template <std::size_t Dimensions, typename Element>
class Vector
{
  public:
    static constexpr std::size_t dimensions = Dimensions;

    Element& operator[](const std::size_t i) noexcept;
    const Element& operator[](const std::size_t i) const noexcept;

  private:
    std::array<Element, Dimensions> m_data;
};

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator+(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept;

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator-(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept;

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator*(const Vector<Dimensions, T>& lhs, const T& rhs) noexcept;

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator-(const Vector<Dimensions, T>& v) noexcept;

template <std::size_t Dimensions, typename T>
bool operator==(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept;

template <std::size_t Dimensions, typename T>
bool operator!=(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept;


template <std::size_t Dimensions, typename T>
T dot_product(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept;

//
// Vector2 declaration.
//

//
// Vector2 declaration.
//

template <typename Element>
class Vector<2, Element>
{
  public:
    static constexpr std::size_t dimensions = 2;

    Vector(Element x, Element y);

    Element& operator[](const std::size_t i) noexcept;
    const Element& operator[](const std::size_t i) const noexcept;

    Element& x() noexcept;
    const Element& x() const noexcept;

    Element& y() noexcept;
    const Element& y() const noexcept;

  private:
    std::array<Element, 2> m_data;
};

template <typename Element>
using Vector2 = Vector<2, Element>;

using Vector2f = Vector2<float>;

//
// Vector3 declaration.
//

template <typename Element>
class Vector<3, Element>
{
  public:
    static constexpr std::size_t dimensions = 3;

    Vector(Element x, Element y, Element z);

    Element& operator[](const std::size_t i) noexcept;
    const Element& operator[](const std::size_t i) const noexcept;

    Element& x() noexcept;
    const Element& x() const noexcept;

    Element& y() noexcept;
    const Element& y() const noexcept;

    Element& z() noexcept;
    const Element& z() const noexcept;

  private:
      std::array<Element, 3> m_data;
};

template <typename Element>
using Vector3 = Vector<3, Element>;

using Vector3f = Vector3<float>;

template <typename T>
Vector3<T> cross_product(const Vector3<T>& lhs, const Vector3<T>& rhs) noexcept;

//
// Vector implementation.
//

template <std::size_t Dimensions, typename T>
T& Vector<Dimensions, T>::operator[](const std::size_t i) noexcept
{
    return m_data[i];
}

template <std::size_t Dimensions, typename T>
const T& Vector<Dimensions, T>::operator[](const std::size_t i) const noexcept
{
    return m_data[i];
}

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator+(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept
{
    auto res = lhs;
    for (std::size_t i = 0; i < Dimensions; ++i)
    {
        res[i] += rhs[i];
    }
    return res;
}

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator-(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept
{
    auto res = lhs;
    for (std::size_t i = 0; i < Dimensions; ++i)
    {
        res[i] -= rhs[i];
    }
    return res;
}

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator*(const Vector<Dimensions, T>& lhs, const T& rhs) noexcept
{
    auto res = lhs;
    for (std::size_t i = 0; i < Dimensions; ++i)
    {
        res[i] *= rhs;
    }
    return res;
}

template <std::size_t Dimensions, typename T>
Vector<Dimensions, T> operator-(const Vector<Dimensions, T>& v) noexcept
{
    return v * (-1.0f);
}

template <std::size_t Dimensions, typename T>
bool operator==(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept
{
    for (std::size_t i = 0; i < Dimensions; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

template <std::size_t Dimensions, typename T>
bool operator!=(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <std::size_t Dimensions, typename T>
T dot_product(const Vector<Dimensions, T>& lhs, const Vector<Dimensions, T>& rhs) noexcept
{
    T res = 0;
    for (std::size_t i = 0; i < Dimensions; ++i)
    {
        res += lhs[i] * rhs[i];
    }
    return res;
}

//
//  Vector2 implementation.
//

template <typename T>
Vector<2, T>::Vector(T x, T y)
  : m_data{ std::move(x), std::move(y) }
{}

template <typename T>
T& Vector<2, T>::operator[](const std::size_t i) noexcept
{
    return m_data[i];
}

template <typename T>
const T& Vector<2, T>::operator[](const std::size_t i) const noexcept
{
    return m_data[i];
}

template <typename T>
T& Vector<2, T>::x() noexcept
{
    return m_data[i];
}

template <typename T>
const T& Vector<2, T>::operator[](const std::size_t& i) const noexcept
{
    return m_data[i];
}

template <typename T>
const T& Vector<2, T>::y() const noexcept
{
    return m_data[1];
}

//
//  Vector3 implementation.
//

template <typename T>
Vector<3, T>::Vector(T x, T y, T z)
  : m_data{std::move(x), std::move(y), std::move(z)}
{}

template <typename T>
T& Vector<3, T>::operator[](const std::size_t i) noexcept
{
    return m_data[i];
}

template <typename T>
const T& Vector<3, T>::operator[](const std::size_t i) const noexcept
{
    return m_data[i];
}

template <typename T>
T& Vector<3, T>::x() noexcept
{
    return m_data[0];
}

template <typename T>
const T& Vector<3, T>::x() const noexcept
{
    return m_data[0];
}

template <typename T>
T& Vector<3, T>::y() noexcept
{
    return m_data[1];
}

template <typename T>
const T& Vector<3, T>::y() const noexcept
{
    return m_data[1];
}

template <typename T>
T& Vector<3, T>::z() noexcept
{
    return m_data[2];
}

template <typename T>
const T& Vector<3, T>::z() const noexcept
{
    return m_data[2];
}

template <typename T>
Vector3<T> cross_product(const Vector3<T>& a, const Vector3<T>& b) noexcept
{
    return Vector3<T> {
        (a.y() * b.z()) - (a.z() * b.y()),
        - (a.x() * b.z()) + (a.z() * b.x()),
        (a.x() * b.y()) - (a.y() * b.x())
    };
}

template <typename T>
const T& Vector<2, T>::y() const noexcept
{
    return m_data[1];
}

//
// Vector3 declaration.
//

template <typename Element>
class Vector<3, Element>
{
  public:
    static constexpr std::size_t dimensions = 3;

    Vector(Element x, Element y, Element z);

    Element& operator[](const std::size_t& i) noexcept;
    const Element& operator[](const std::size_t& i) const noexcept;

    Element& x() noexcept;
    const Element& x() const noexcept;

    Element& y() noexcept;
    const Element& y() const noexcept;

    Element& z() noexcept;
    const Element& z() const noexcept;

  private:
      std::array<Element, 3> m_data;
};

template <typename Element>
using Vector3 = Vector<3, Element>;

using Vector3f = Vector3<float>;

template <typename T>
Vector3<T> cross_product(const Vector3<T>& lhs, const Vector3<T>& rhs) noexcept;

//
//  Vector3 implementation.
//

template <typename T>
Vector<3, T>::Vector(T x, T y, T z)
  : m_data{std::move(x), std::move(y), std::move(z)}
{}

template <typename T>
T& Vector<3, T>::operator[](const std::size_t& i) noexcept
{
    return m_data[i];
}

template <typename T>
const T& Vector<3, T>::operator[](const std::size_t& i) const noexcept
{
    return m_data[i];
}

template <typename T>
T& Vector<3, T>::x() noexcept
{
    return m_data[0];
}

template <typename T>
const T& Vector<3, T>::x() const noexcept
{
    return m_data[0];
}

template <typename T>
T& Vector<3, T>::y() noexcept
{
    return m_data[1];
}

template <typename T>
const T& Vector<3, T>::y() const noexcept
{
    return m_data[1];
}

template <typename T>
T& Vector<3, T>::z() noexcept
{
    return m_data[2];
}

template <typename T>
const T& Vector<3, T>::z() const noexcept
{
    return m_data[2];
}

template <typename T>
Vector3<T> cross_product(const Vector3<T>& a, const Vector3<T>& b) noexcept
{
    return Vector3<T> {
        (a.y() * b.z()) - (a.z() * b.y()),
        - (a.x() * b.z()) + (a.z() * b.x()),
        (a.x() * b.y()) - (a.y() * b.x())
    };
}

} // namespace foundation
