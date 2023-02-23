/*
 * C S 2 E X P S P . I P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of inline non-template methods and inline global functions

#include "base/diag.hpp"

#include "profiler/stktrace.hpp"

#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif

//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
const PhysConfigSpace2d& PhysCS2dExpansionSpace::configSpace() const
{
    return configSpace_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
PhysConfigSpace2d* PhysCS2dExpansionSpace::pConfigSpace()
{
    return &configSpace_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
size_t PhysCS2dExpansionSpace::nClients() const
{
    return nClients_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
void PhysCS2dExpansionSpace::addClient()
{
    CS2VGRA_WHERE;
    CS2VGRA_STREAM(ProStackTracer());

    ++nClients_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
void PhysCS2dExpansionSpace::removeClient()
{
    CS2VGRA_WHERE;
    CS2VGRA_STREAM(ProStackTracer());

    PRE(nClients() > 0);
    --nClients_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
MATHEX_SCALAR PhysCS2dExpansionSpace::expansionDistance() const
{
    return expansionDistance_;
}
//////////////////////////////////////////////////////////////////////////////////////////
/* End CS2EXPSP.IPP *************************************************/
