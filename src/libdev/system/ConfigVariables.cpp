#include "ConfigVariables.hpp"

namespace Config
{

Variable<int32_t> gfxMsaaBuffers("Options/MSAA/Buffers", 1);
Variable<int32_t> gfxMsaaSamples("Options/MSAA/Samples", 2);
Variable<int32_t> uiScaleFactor("Options/Scale Factor/Value", 0);
Variable<bool> grabCursor("Options/Grab Cursor/on", true);

} // namespace Config
