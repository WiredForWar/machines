#pragma once

#include "ctl/TFlag.hpp"

namespace Render
{

// Use a namespace to import it in other namespaces (e.g. Gui)
namespace Align
{
    enum AlignmentFlag
    {
        AlignNone,
        AlignLeft = 0x01,
        AlignRight = 0x02,
    };

} // namespace Align

using namespace Align;
using Alignment = TFlag<AlignmentFlag>;

} // namespace Render
