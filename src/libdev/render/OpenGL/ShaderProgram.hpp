#pragma once

#include <string>
#include <string_view>

#include <GL/glew.h>

namespace Ren
{

namespace OpenGL
{

// Compile a vertex and a fragment shader and link the two into a program, or
// return 0 having put on the log whatever the driver objected to.
//
// Shared by every GL backend, because none of these calls changed: compiling
// and linking work the same way in 2.1, in core and in ES, and the only thing
// that differs between them is the dialect of the sources handed in.
//
// The debug names name the shaders the sources were read from, and are used for
// nothing but diagnostics.
GLuint createProgramFromSources(
    const std::string& vertexShaderCode,
    const std::string& fragmentShaderCode,
    std::string_view vertexShaderDebugName,
    std::string_view fragmentShaderDebugName);

} // namespace OpenGL

} // namespace Ren
