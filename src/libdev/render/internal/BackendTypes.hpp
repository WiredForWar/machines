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
struct CommandBufferTag;

template <typename Tag>
class LocationId
{
public:
    constexpr LocationId() = default;

    explicit constexpr LocationId(int value)
        : value_{ value }
    {
    }

    [[nodiscard]] constexpr int value() const { return value_; }

    [[nodiscard]] constexpr bool isValid() const { return value_ >= 0; }

    constexpr bool operator==(const LocationId& other) const = default;

private:
    int value_{-1};
};

struct UniformTag;
struct AttributeTag;

} // namespace internal

using BackendTextureHandle = internal::BackendHandle<internal::TextureTag>;
using BackendCommandBufferHandle = internal::BackendHandle<internal::CommandBufferTag>;

using UniformLocationId = internal::LocationId<internal::UniformTag>;
using AttributeLocationId = internal::LocationId<internal::AttributeTag>;

enum class BufferTarget
{
    Array,
    ElementArray,
};

enum class BufferUsage
{
    StreamDraw,
};

enum class TextureFormat
{
    RGBA8_UNorm,
    Depth16,
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

enum class BackendClearFlag : std::uint32_t
{
    Colour = 1u << 0,
    Depth = 1u << 1,
    Stencil = 1u << 2,
};

constexpr std::uint32_t backendClearMask(BackendClearFlag flag);
constexpr std::uint32_t operator|(BackendClearFlag flag1, BackendClearFlag flag2);

enum class BackendIndexType
{
    UnsignedByte,
    UnsignedShort,
    UnsignedInt,
};

enum class BackendBlendFactor
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
};

enum class BackendDepthFunc
{
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
};

enum class BackendVertexAttribType
{
    Float,
    UnsignedByte,
};

inline constexpr std::uint32_t backendClearMask(BackendClearFlag flag)
{
    return static_cast<std::uint32_t>(flag);
}

inline constexpr std::uint32_t operator|(BackendClearFlag flag1, BackendClearFlag flag2)
{
    return static_cast<std::uint32_t>(flag1) | static_cast<std::uint32_t>(flag2);
}

} // namespace Ren
