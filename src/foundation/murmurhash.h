#pragma once

#include <cstddef>
#include <cstdint>

namespace foundation
{

/**
 *  Based on SMHasher code.
 *  "All MurmurHash versions are public domain software, and the author disclaims all copyright to their code."
 */
class murmur_hash
{
  public:
    murmur_hash() noexcept;

    void append(const void* payload, std::size_t len) noexcept;
    
    std::size_t as_64bit() const noexcept;

  private:
    std::uint64_t m_h1;
    std::uint64_t m_h2;
};

/**
 *  Hasher object allows using (murmur-)hashable classes
 *  as hash map keys.
 */
template <typename T>
struct hasher
{
    std::size_t operator()(const T& value) const noexcept
    {
        return value.hash().as_64bit();
    }
};

} // namespace foundation
