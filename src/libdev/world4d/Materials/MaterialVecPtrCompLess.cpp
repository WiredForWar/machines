/*
 * M V E C C P L E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "ctl/CountedPtr.hpp"

#include "world4d/Materials/MaterialVecPtrCompLess.hpp"

#include "render/MaterialVec.hpp"

bool W4dMaterialVecPtr_SetLess::operator()(const Ren::MaterialVecPtr& lhs, const Ren::MaterialVecPtr& rhs) const
{
    return (*lhs) < (*rhs);
}
/* End MVECCPLE.CPP *************************************************/
