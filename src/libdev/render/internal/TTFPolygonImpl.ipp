/*
 * T T F P O L Y I . I P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

#include "render/internal/DeviceImpl.hpp"
#include "render/Device.hpp"

#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif

_CODE_INLINE
void RenITTFImpl::renderGL(RenIVertex* vtx, const RenMaterial& mat, Ren::PrimitiveTopology topology) const
{
    RenDevice::current()->recordCommand(Ren::Command::setCullFace(false));
    Ren::VertexIdx* crufty = const_cast<Ren::VertexIdx*>(indices_);
    RenDevice::current()->renderIndexedScreenspace(vtx, nVertices_, crufty, nIndices_, mat, topology);
    RenDevice::current()->recordCommand(Ren::Command::setCullFace(true));
}

/* End TTFPOLYI.IPP *************************************************/
