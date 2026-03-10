/*
 * M A T M A P . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#pragma once

#include "render/Material.hpp"

#include <set>
#include <map>

// Note: less<RenMaterial> isn't used simply to reduce STL dependencies.
using RenMaterialSet = std::set<RenMaterial>;
using RenMaterialMap = std::map<RenMaterial, RenMaterial>;
