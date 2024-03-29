/*
 * I N T E R N A L . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_INTERNAL_HPP
#define _RENDER_INTERNAL_HPP

#include "base/base.hpp"

#include <memory>

class RenIVertex;

// Various widely used typedefs.  These could correctly be put in class
// scopes, e.g. RenTexManager::TexId.  However, putting them here helps
// reduce dependencies.
// Hence, this file is supposed to be **small & simple**.
namespace RenI {
using LitVtxAPtr = std::unique_ptr<RenIVertex[]>;
enum DisplayType
{
    FRONT,
    BACK,
    NOT_DISPLAY
};
enum UpdateType
{
    FORCE_UPDATE,
    LAZY_UPDATE
};
}; // namespace RenI

#endif

/* End INTERNAL.HPP ***************************************************/
