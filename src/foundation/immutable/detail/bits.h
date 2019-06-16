#pragma once

#include <cstddef>
#include <cstdint>
#ifdef _WIN32
#include <intrin.h>
#endif

namespace foundation
{
namespace immutable
{
namespace detail
{

using BitmapType = std::uint64_t;
using CountType = std::uint32_t;
using ShiftType = std::uint32_t;
using HashType = std::size_t;

template <CountType B, typename T = std::size_t>
constexpr T max_depth = (sizeof(HashType) * 8u + B - 1u) / B;

template <CountType B, typename T = ShiftType>
constexpr T max_shift = max_depth<B, T> * B;

template <CountType B, typename T = BitmapType>
constexpr T mask = (1ul << B) - 1ul;

inline CountType popcount(std::uint64_t x)
{
#ifdef _WIN32
	return __popcnt64(x);
#else
    return __builtin_popcountll(x);
#endif
}

} // namespace detail
} // namespace immutable
} // namespace foundation
