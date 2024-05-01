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
        AlignHCenter = 0x04,
        AlignTop = 0x08,
        AlignBottom = 0x10,
        AlignVCenter = 0x20,
        AlignCenter = AlignHCenter | AlignVCenter,

        AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter,
        AlignVertical_Mask = AlignTop | AlignBottom | AlignVCenter,
    };

} // namespace Align

using namespace Align;
using Alignment = TFlag<AlignmentFlag>;

} // namespace Render
