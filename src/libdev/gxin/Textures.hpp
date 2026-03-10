/*
 * G X T E X T U R E S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    GXTextures

*/

#ifndef _GXTEXTURES_HPP
#define _GXTEXTURES_HPP

#include "base/base.hpp"
#include "ctl/Map.hpp"

#include "gxin/GXTexture.hpp"
#include "gxin/IdPos.hpp"

using GXTextures = ctl_map<GXIdPos, GXTexture, std::less<GXIdPos>>;

#endif

/* End GXTEXTURES.HPP ***********************************************/
