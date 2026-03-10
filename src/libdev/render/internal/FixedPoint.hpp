/*
 * F I X P T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    Simple, non-class, fixed-point types used internally.
*/

#ifndef _RENDER_FIXPT_HPP
#define _RENDER_FIXPT_HPP

#include "base/base.hpp"

class RenColour;

using RenIFixPtU0_8 = unsigned char;
using RenIFixPtS0_7 = signed char;
using RenIFixPtS23_8 = int;
using RenIFixPtS15_16 = int;

struct RenIColourFixPtS15_16
{
    RenIColourFixPtS15_16();
    RenIColourFixPtS15_16(const RenColour&);
    RenIFixPtS15_16 r_, g_, b_;
};

struct RenIColourFixPtS23_8
{
    RenIColourFixPtS23_8();
    RenIColourFixPtS23_8(const RenColour&);
    RenIFixPtS23_8 r_, g_, b_;
};

#endif

/* End FIXPT.HPP ****************************************************/
