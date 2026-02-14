#pragma once

#include "render/internal/BackendTypes.hpp"

#include <cstdint>

namespace Ren
{

using RenderPassId = std::uint32_t;

enum class LoadOp
{
    Load,
    Clear,
    DontCare,
};

enum class StoreOp
{
    Store,
    DontCare,
};

struct AttachmentDesc
{
    LoadOp loadOp{LoadOp::DontCare};
    StoreOp storeOp{StoreOp::Store};
    float clearR{};
    float clearG{};
    float clearB{};
    float clearA{};
};

struct RenderPassDesc
{
    AttachmentDesc colorAttachment{};
    AttachmentDesc depthAttachment{};
    bool hasDepthAttachment{};
};

} // namespace Ren
