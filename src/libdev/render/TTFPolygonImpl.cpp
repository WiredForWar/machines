/*
 * T T F P O L Y I . C P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

#include "render/internal/TTFPolygonImpl.hpp"
#include "render/internal/VertexData.hpp"
#include "mathex/Vec2.hpp"

#ifndef _INLINE
#include "render/internal/TTFPolygonImpl.ipp"
#endif

#include "render/Colour.hpp" // temporary

RenITTFImpl::RenITTFImpl(size_t nVertices, const Ren::VertexIdx* idx, size_t nIndices)
    : vertices_(_NEW_ARRAY(RenIVertex, nVertices))
    , uvs_(_NEW_ARRAY(MexVec2, nVertices))
    , nVertices_(nVertices)
    , indices_(idx)
    , nIndices_(nIndices)
{
}

RenITTFImpl::RenITTFImpl(const RenITTFImpl& rhs)
    : vertices_(_NEW_ARRAY(RenIVertex, rhs.nVertices_))
    , uvs_(_NEW_ARRAY(MexVec2, rhs.nVertices_))
    , nVertices_(rhs.nVertices_)
    , indices_(rhs.indices_)
    , nIndices_(rhs.nIndices_)
    , material_(rhs.material_)
{
    for (int i = 0; i != nVertices_; ++i)
        uvs_[i] = rhs.uvs_[i];
}

RenITTFImpl::~RenITTFImpl()
{
    _DELETE_ARRAY(vertices_);
    _DELETE_ARRAY(uvs_);
}

/* End TTFPOLYI.CPP *************************************************/
