#include "foundation/murmurhash.h"

#include <cstdint>

namespace foundation
{

namespace
{

inline std::uint64_t rotl64(std::uint64_t x, std::int8_t r)
{
    return (x << r) | (x >> (64 - r));
}

inline std::uint64_t fmix64(std::uint64_t k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccd;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53;
    k ^= k >> 33;

    return k;
}

} // namespace

MurmurHash::MurmurHash() noexcept
    : m_h1(0u)
    , m_h2(0u)
{}

void MurmurHash::append(const void* payload, std::size_t len) noexcept
{
    const std::uint8_t* data = reinterpret_cast<const std::uint8_t*>(payload);
    const std::size_t nblocks = len / 16;

    constexpr uint64_t c1 = 0x87c37b91114253d5;
    constexpr uint64_t c2 = 0x4cf5ad432745937f;

    // Body.
    const std::uint64_t* blocks = reinterpret_cast<const std::uint64_t*>(data);

    for(std::size_t i = 0; i < nblocks; ++i)
    {
        std::uint64_t k1 = blocks[i * 2];
        std::uint64_t k2 = blocks[i * 2 + 1];

        k1 *= c1;
        k1 = rotl64(k1, 31);
        k1 *= c2;
        m_h1 ^= k1;

        m_h1 = rotl64(m_h1, 27);
        m_h1 += m_h2;
        m_h1 = m_h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = rotl64(k2, 33);
        k2 *= c1;
        m_h2 ^= k2;

        m_h2 = rotl64(m_h2, 31);
        m_h2 += m_h1;
        m_h2 *= 5 + 0x38495ab5;
    }

    // Tail.
    const std::uint8_t* tail = data + nblocks * 16;

    std::uint64_t k1 = 0;
    std::uint64_t k2 = 0;

    switch(len & 15)
    {
        case 15: k2 ^= (static_cast<std::uint64_t>(tail[14])) << 48;
        case 14: k2 ^= (static_cast<std::uint64_t>(tail[13])) << 40;
        case 13: k2 ^= (static_cast<std::uint64_t>(tail[12])) << 32;
        case 12: k2 ^= (static_cast<std::uint64_t>(tail[11])) << 24;
        case 11: k2 ^= (static_cast<std::uint64_t>(tail[10])) << 16;
        case 10: k2 ^= (static_cast<std::uint64_t>(tail[ 9])) << 8;
        case  9: k2 ^= (static_cast<std::uint64_t>(tail[ 8])) << 0;
                    k2 *= c2;
                    k2 = rotl64(k2, 33);
                    k2 *= c1;
                    m_h2 ^= k2;

        case  8: k1 ^= (static_cast<std::uint64_t>(tail[ 7])) << 56;
        case  7: k1 ^= (static_cast<std::uint64_t>(tail[ 6])) << 48;
        case  6: k1 ^= (static_cast<std::uint64_t>(tail[ 5])) << 40;
        case  5: k1 ^= (static_cast<std::uint64_t>(tail[ 4])) << 32;
        case  4: k1 ^= (static_cast<std::uint64_t>(tail[ 3])) << 24;
        case  3: k1 ^= (static_cast<std::uint64_t>(tail[ 2])) << 16;
        case  2: k1 ^= (static_cast<std::uint64_t>(tail[ 1])) << 8;
        case  1: k1 ^= (static_cast<std::uint64_t>(tail[ 0])) << 0;
                    k1 *= c1;
                    k1 = rotl64(k1, 31);
                    k1 *= c2;
                    m_h1 ^= k1;
    };

    // Finalize.
    m_h1 ^= len;
    m_h2 ^= len;

    m_h1 += m_h2;
    m_h2 += m_h1;

    m_h1 = fmix64(m_h1);
    m_h2 = fmix64(m_h2);

    m_h1 += m_h2;
    m_h2 += m_h1;
}

std::size_t MurmurHash::as_64bit() const noexcept
{
    return m_h1 ^ (m_h2 + 0x9e3779b9 + (m_h1 << 6) + (m_h1 >> 2));
}

} // namespace foundation
