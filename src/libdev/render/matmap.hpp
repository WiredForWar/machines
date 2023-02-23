/*
 * M A T M A P . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_MATMAP_HPP
#define _RENDER_MATMAP_HPP

#include "ctl/set.hpp"
#include "ctl/map.hpp"
#include "render/material.hpp"

// Note: less<RenMaterial> isn't used simply to reduce STL dependencies.
using RenMaterialSet = ctl_set<RenMaterial, RenMaterial::Less>;
using RenMaterialMap = ctl_map<RenMaterial, RenMaterial, RenMaterial::Less>;

#endif

/* End MATMAP.HPP ***************************************************/
