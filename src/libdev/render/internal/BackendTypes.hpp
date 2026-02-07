#pragma once

#include <cstdint>

namespace Ren
{

namespace internal
{

template <typename Tag>
class BackendHandle
{
public:
    constexpr BackendHandle() = default;

    explicit constexpr BackendHandle(std::uint32_t value)
        : value_{ value }
    {
    }

    [[nodiscard]] constexpr std::uint32_t value() const { return value_; }

    [[nodiscard]] constexpr bool isValid() const { return value_ != 0; }

    constexpr bool operator==(const BackendHandle& other) const = default;

private:
    std::uint32_t value_{};
};

struct TextureTag;

} // namespace internal

using BackendTextureHandle = internal::BackendHandle<internal::TextureTag>;

enum class TextureFormat
{
    RGBA8_UNorm,
};

enum class TextureFilter
{
    Nearest,
    Linear,
    LinearMipmapLinear,
};

enum class TextureWrap
{
    Repeat,
    ClampToEdge,
};

} // namespace Ren
