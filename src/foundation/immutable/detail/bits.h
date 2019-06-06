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

using bitmap_t = std::uint64_t;
using count_t = std::uint32_t;
using shift_t = std::uint32_t;
using hash_t = std::size_t;

template <count_t B, typename T = std::size_t>
constexpr T max_depth = (sizeof(hash_t) * 8u + B - 1u) / B;

template <count_t B, typename T = shift_t>
constexpr T max_shift = max_depth<B, T> * B;

template <count_t B, typename T = bitmap_t>
constexpr T mask = (1ul << B) - 1ul;

inline count_t popcount(std::uint64_t x)
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
