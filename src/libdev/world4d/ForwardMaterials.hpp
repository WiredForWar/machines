/*
 * F W M A T E R I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    Forward declarations for classes related to materials and vectors of em etc
*/

#ifndef _WORLD4D_FWMATERI_HPP
#define _WORLD4D_FWMATERI_HPP

#include "render/render.hpp"
#include "render/material.hpp"
#include "render/matmap.hpp"

template <class T> class ctl_vector;
template <class KEY, class T, class COMPARE> class ctl_map;

using W4dMaterialVecPtrs = ctl_vector<Ren::MaterialVecPtr>;
class W4dMaterialVecPtr_SetLess;

// A set of counted pointers to material vectors, sorted on the material vector value
// rather than pointer address.
using W4dMaterialVecPtrSet = std::set<Ren::MaterialVecPtr>;

#endif

/* End FWMATERI.HPP *************************************************/
