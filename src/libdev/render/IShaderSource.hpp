#pragma once

#include "render/ShaderSet.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace Ren
{

enum class ShaderStage
{
    Vertex,
    Fragment,
};

// Where a backend gets the text of its shaders. The backend knows which dialect
// it can compile and which stages it needs; where that text comes from, and
// whether it is a loose file, a mod override or an entry in an archive, is the
// host's to answer.
class IShaderSource
{
public:
    virtual ~IShaderSource() = default;

    // The source of one shader, or nothing when the set does not have it. A set
    // may provide the shaders for some effects and not others, so absence is an
    // answer rather than a failure, and it is for the caller to decide whether
    // it can go without.
    virtual std::optional<std::string> source(ShaderSet set, ShaderStage stage, std::string_view name) const = 0;
};

} // namespace Ren
