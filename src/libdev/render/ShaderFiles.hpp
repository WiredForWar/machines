#pragma once

#include "render/IShaderSource.hpp"

namespace Ren
{

// Shaders read from the data directory, through the same path resolution as the
// rest of the game's data, so that a mod can put its own in front of them.
class ShaderFiles final : public IShaderSource
{
public:
    std::optional<std::string> source(ShaderSet set, ShaderStage stage, std::string_view name) const override;
};

} // namespace Ren
