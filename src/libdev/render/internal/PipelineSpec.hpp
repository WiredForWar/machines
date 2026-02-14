#pragma once

#include "render/internal/BackendTypes.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Ren
{

using PipelineId = std::uint32_t;

struct VertexAttributeDesc
{
    std::string name{};
    int componentCount{};
    BackendVertexAttribType type{BackendVertexAttribType::Float};
    bool normalized{};
    std::size_t stride{};
    std::size_t offset{};
};

struct PipelineDesc
{
    std::string vertexShader{};
    std::string fragmentShader{};
    std::vector<VertexAttributeDesc> vertexAttributes{};
    std::vector<std::string> uniformNames{};
};

} // namespace Ren
