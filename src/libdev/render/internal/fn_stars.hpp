/***********************************************************

  F N _ S T A R S . H P P
  (c) Charybdis Limited, 1998. All Rights Reserved

***********************************************************/

/*
  There are quite a lot of functions and functors in here to help keep the stars fast.
*/

#ifndef _INTERNAL_RENDER_STARS_IMPL_FUNCTORS_HPP
#define _INTERNAL_RENDER_STARS_IMPL_FUNCTORS_HPP

#include "mathex/mathex.hpp"
#include "mathex/random.hpp"
#include "mathex/vec3.hpp"
#include "mathex/epsilon.hpp"

#include "render/render.hpp"
#include "render/colour.hpp"
#include "render/internal/colpack.hpp"
#include "render/internal/vtxdata.hpp"
#include "render/device.hpp"

#include "ctl/algorith.hpp"
#include "ctl/vector.hpp"

#include <numeric>
#include <iterator>
#include <functional>
#include <algorithm>

#include "utility/percent.hpp"

////////////////////////////////////////////////////////////
class RenIVertex;

void renIStarsImplReserveVectorsOp(ctl_vector<RenIVertex>& sector);

class RenIStarsImplD3DLVERTEXGen;
class RenIStarsImplSphericalD3DLVERTEXGen;
class RenIStarsImplHemisphericalD3DLVERTEXGen;

class RenIStarsImplD3DLVERTEXInserterOp;


class RenIStarsImplD3DLVERTEXRandomiseAlphasAuxOp;
class RenIStarsImplD3DLVERTEXRandomiseAlphasOp;

class RenIStarsImplVerticesColourOp;
class RenIStarsImplVerticesColourAuxOp;

class RenIStarsImplDefineNorthCapOp;
class RenIStarsImplDefineSouthCapOp;

class RenIStarsImplSectorClipBinPred;

class RenIStarsImplPegStarsOp;

////////////////////////////////////////////////////////////

void RenIStarsImplReserveVectorsOp(ctl_vector<RenIVertex>& sector)
{
    // This value is for an average hemisphere of 2,000 stars with 16 sectors.
    sector.reserve(125);
}

////////////////////////////////////////////////////////////

class RenIStarsImplD3DLVERTEXGen
{
public:
    virtual RenIVertex operator()() = 0;
};

class RenIStarsImplSphericalD3DLVERTEXGen : public RenIStarsImplD3DLVERTEXGen
{
public:
    RenIStarsImplSphericalD3DLVERTEXGen(MATHEX_SCALAR radius, RenColour rgbColour)
        : radius_(radius)
        , rgbColour_(rgbColour)
        , random_(MexBasicRandom::constructSeededFromTime())
    {
    }

    RenIVertex operator()() override
    {
        MexVec3 vec(
            mexRandomScalar(&random_, -1.0, 1.0),
            mexRandomScalar(&random_, -1.0, 1.0),
            mexRandomScalar(&random_, -1.0, 1.0));

        vec.makeUnitVector();
        vec *= radius_;

        RenIVertex d3dLVertex;
        d3dLVertex.x = vec.x();
        d3dLVertex.y = vec.y();
        d3dLVertex.z = vec.z();
        d3dLVertex.color = packColour(rgbColour_.r(), rgbColour_.b(), rgbColour_.b(), 1.0);
        d3dLVertex.specular = 0;

        return d3dLVertex;
    }

private:
    MATHEX_SCALAR radius_;
    RenColour rgbColour_;
    MexBasicRandom random_;
};

class RenIStarsImplHemisphericalD3DLVERTEXGen : public RenIStarsImplD3DLVERTEXGen
{
public:
    RenIStarsImplHemisphericalD3DLVERTEXGen(MATHEX_SCALAR radius, RenColour rgbColour)

        : radius_(radius)
        , rgbColour_(rgbColour)
        , random_(MexBasicRandom::constructSeededFromTime())
    {
    }

    RenIVertex operator()() override
    {
        MexVec3 vec(
            mexRandomScalar(&random_, -1.0, 1.0),
            mexRandomScalar(&random_, -1.0, 1.0),
            mexRandomScalar(&random_, 0.0, 1.0));

        vec.makeUnitVector();
        vec *= radius_;

        RenIVertex d3dLVertex;
        d3dLVertex.x = vec.x();
        d3dLVertex.y = vec.y();
        d3dLVertex.z = vec.z();
        d3dLVertex.color = packColour(rgbColour_.r(), rgbColour_.b(), rgbColour_.b(), 1.0);
        d3dLVertex.specular = 0;
        return d3dLVertex;
    }

private:
    MATHEX_SCALAR radius_;
    RenColour rgbColour_;
    MexBasicRandom random_;
};

////////////////////////////////////////////////////////////

class RenIStarsImplD3DLVERTEXInserterOp
{
public:
    RenIStarsImplD3DLVERTEXInserterOp(
        ctl_vector<ctl_vector<RenIVertex>>* const pSectors,
        int nSectors,
        MATHEX_SCALAR sectorWidth)

        : pSectors_(pSectors)
        , nSectors_(nSectors)
        , sectorWidth_(sectorWidth)
    {
    }

    void operator()(const RenIVertex& vertex) const
    {
        // The azimuthal angle of the vector.
        MATHEX_SCALAR azi = atan2(vertex.y, vertex.x);

        // Place the vertex in the currect sector.
        MATHEX_SCALAR sectorPartition = sectorWidth_ - Mathex::PI;

        for (int sectorIndex = 0; sectorIndex < nSectors_; ++sectorIndex)
        {
            if (azi < sectorPartition)
            {
                (*pSectors_)[sectorIndex].push_back(vertex);
                break;
            }

            sectorPartition += sectorWidth_;
        }
    }

private:
    ctl_vector<ctl_vector<RenIVertex>>* const pSectors_;
    int nSectors_;
    MATHEX_SCALAR sectorWidth_;
};

////////////////////////////////////////////////////////////
/* */
inline bool RenIStarsImplVertexHeightLesserOrEqual(const RenIVertex& lhs, const RenIVertex& rhs)
{
    return lhs.z <= rhs.z;
}

////////////////////////////////////////////////////////////

class RenIStarsImplD3DLVERTEXRandomiseAlphasAuxOp
{
public:
    RenIStarsImplD3DLVERTEXRandomiseAlphasAuxOp(MATHEX_SCALAR lowerBound, MATHEX_SCALAR upperBound)
        : random_(MexBasicRandom::constructSeededFromTime())
        , lowerBound_(lowerBound)
        , upperBound_(upperBound)
    {
    }

    uint setD3DCOLORAlpha(uint colour, MATHEX_SCALAR alpha) const
    {
        PRE(alpha >= 0.0 && alpha <= 1.0);

        return (colour & 0x00ffffff) | (_STATIC_CAST(uint, alpha * 255) << 24);
    }

    void operator()(RenIVertex& vertex)
    {
        MATHEX_SCALAR randAlpha = mexRandomScalar(&random_, lowerBound_, upperBound_);

        vertex.color = setD3DCOLORAlpha(vertex.color, randAlpha);
    }

private:
    MexBasicRandom random_;
    MATHEX_SCALAR lowerBound_;
    MATHEX_SCALAR upperBound_;
};

class RenIStarsImplD3DLVERTEXRandomiseAlphasOp
{
public:
    RenIStarsImplD3DLVERTEXRandomiseAlphasOp(MATHEX_SCALAR lowerBound, MATHEX_SCALAR upperBound)

        : alphaRandomiser_(lowerBound, upperBound)
    {
        PRE_INFO(lowerBound);
        PRE_INFO(upperBound);
        PRE(lowerBound < upperBound);
        PRE(inIntervalOO(lowerBound, 0.0, 1.0));
        PRE(inIntervalOO(upperBound, 0.0, 1.0));
    }

    void operator()(ctl_vector<RenIVertex>& sector) const { ctl_for_each(sector, alphaRandomiser_); }

private:
    RenIStarsImplD3DLVERTEXRandomiseAlphasAuxOp alphaRandomiser_;
};

////////////////////////////////////////////////////////////

class RenIStarsImplVerticesColourAuxOp
{
public:
    RenIStarsImplVerticesColourAuxOp(uint colour)
        : colour_(colour)
    {
    }

    void operator()(RenIVertex& vertex)
    {
        // Take the current alpha and set the RGB.
        vertex.color = (vertex.color & 0xff000000) | (colour_ & 0x00ffffff);
    }

private:
    uint colour_{};
};

class RenIStarsImplVerticesColourOp
{
public:
    RenIStarsImplVerticesColourOp(RenColour colour)
    {
        // NVG uses over-brighting so on this colour so make sure it's in a sensible boundary.
        colour_ = packColour(
            mexClamp(colour.r(), 0.0f, 1.0f),
            mexClamp(colour.g(), 0.0f, 1.0f),
            mexClamp(colour.b(), 0.0f, 1.0f),
            1.0);
    }

    void operator()(ctl_vector<RenIVertex>& sector) { ctl_for_each(sector, RenIStarsImplVerticesColourAuxOp(colour_)); }

private:
    uint colour_{};
};

////////////////////////////////////////////////////////////

class RenIStarsImplDefineNorthCapOp
{
public:
    RenIStarsImplDefineNorthCapOp(
        MATHEX_SCALAR rimHeight,
        ctl_vector<RenIVertex*>* pVertexPtrs,
        ctl_vector<int>* pSizes)

        : rimHeight_(rimHeight)
        , pVertexPtrs_(pVertexPtrs)
        , pSizes_(pSizes)
    {
        PRE(rimHeight_ > 0.0);
        PRE(pVertexPtrs_);
        PRE(pVertexPtrs_->empty());
        // PRE(pVertexPtrs_->capacity() > 0);
        PRE(pSizes_);
        PRE(pSizes_->empty());
        // PRE(pSizes_->capacity() > 0);
    }

    void operator()(ctl_vector<RenIVertex>& sector) const
    {
        auto rimHeight = rimHeight_;
        ctl_vector<RenIVertex>::iterator itRim = std::find_if(
            sector.begin(),
            sector.end(),
            [rimHeight](const RenIVertex& vertex) { return vertex.z >= rimHeight; });

        ASSERT(itRim != sector.end(), "There are no stars in the north cap.");

        ptrdiff_t size = 0;
        // TODO check std::distance(itRim, sector.end(), size);
        size = std::distance(itRim, sector.end());

        ASSERT(size > 0, "Attempted to render an array of vertices zero elements long");

        (*pVertexPtrs_).push_back(&(*itRim));
        pSizes_->push_back(size);
    }

private:
    MATHEX_SCALAR rimHeight_;
    ctl_vector<RenIVertex*>* const pVertexPtrs_;
    ctl_vector<int>* const pSizes_;
};

class RenIStarsImplDefineSouthCapOp
{
public:
    RenIStarsImplDefineSouthCapOp(
        MATHEX_SCALAR rimHeight,
        ctl_vector<RenIVertex*>* pVertexPtrs,
        ctl_vector<int>* pSizes)

        : rimHeight_(rimHeight)
        , pVertexPtrs_(pVertexPtrs)
        , pSizes_(pSizes)
    {
        PRE(rimHeight_ < 0.0);
        PRE(pVertexPtrs_);
        PRE(pVertexPtrs_->empty());
        // PRE(pVertexPtrs_->capacity() > 0);
        PRE(pSizes_);
        PRE(pSizes_->empty());
        // PRE(pSizes_->capacity() > 0);
    }

    // I would like this to be a const parameter as I don't change the sequence at all.
    // I do however put a pointer to an element into another sequence. This means that
    // the type parameter to that sequence would need to be const D3DLVERTEX* and this
    // breaks when a const_iterator is instantiated (if the iterator is a typedefed pointer).
    //  void operator ()(ctl_vector<D3DLVERTEX>& sector) const
    void operator()(ctl_vector<RenIVertex>& sector) const
    {
        auto rimHeight = rimHeight_;
        ctl_vector<RenIVertex>::iterator itRim = std::find_if(
            sector.begin(),
            sector.end(),
            [rimHeight](const RenIVertex& vertex) { return vertex.z >= rimHeight; });

        ASSERT(itRim != sector.end(), "There are no stars in the south cap.");

        ptrdiff_t size = 0;
        // std::distance(sector.begin(), itRim, size);
        size = std::distance(sector.begin(), itRim);

        ASSERT(size > 0, "Attempted to render an array of vertices zero elements long");

        pVertexPtrs_->push_back(&(*sector.begin()));
        pSizes_->push_back(size);
    }

private:
    MATHEX_SCALAR rimHeight_;
    ctl_vector<RenIVertex*>* const pVertexPtrs_;
    ctl_vector<int>* const pSizes_;
};

////////////////////////////////////////////////////////////

class RenIStarsImplSectorClipBinPred
{
public:
    RenIStarsImplSectorClipBinPred(MATHEX_SCALAR antiRay, MATHEX_SCALAR clockRay)
        : antiRay_(antiRay)
        , clockRay_(clockRay)
    {
    }

    bool operator()(MATHEX_SCALAR antiPartition, MATHEX_SCALAR clockPartition) const
    {
        bool result = false;

        if (antiRay_ > clockRay_)
        {
            if (!inIntervalOO(antiPartition, clockRay_, antiRay_) || !inIntervalOO(clockPartition, clockRay_, antiRay_))
                result = true;
        }
        else
        {
            if (inIntervalOO(antiPartition, antiRay_, clockRay_) || inIntervalOO(clockPartition, antiRay_, clockRay_))
                result = true;
        }

        return result;
    }

private:
    MATHEX_SCALAR antiRay_;
    MATHEX_SCALAR clockRay_;
};

////////////////////////////////////////////////////////////

class RenIStarsImplPegStarsOp
{
public:
    RenIStarsImplPegStarsOp(
        ctl_vector<RenIVertex*>* const pVertexPtrs,
        ctl_vector<int>* const pSizes,
        float minHeight,
        float maxHeight)

        : pVertexPtrs_(pVertexPtrs)
        , pSizes_(pSizes)
        , minHeight_(minHeight)
        , maxHeight_(maxHeight)
    {
    }

    void operator()(ctl_vector<RenIVertex>* const pSector) const
    {

        RenIVertex minVertex;
        minVertex.x = minVertex.y = 0;
        minVertex.z = minHeight_;
        minVertex.color = 0;
        minVertex.specular = 0;
        RenIVertex maxVertex;
        maxVertex.x = maxVertex.y = 0;
        maxVertex.z = maxHeight_;
        maxVertex.color = 0;
        maxVertex.specular = 0;

        auto minHeight = minHeight_;
        ctl_vector<RenIVertex>::iterator itMin = std::find_if(
            pSector->begin(),
            pSector->end(),
            [minHeight](const RenIVertex& vertex) { return vertex.z >= minHeight; });

        auto maxHeight = maxHeight_;
        ctl_vector<RenIVertex>::iterator itMax = std::find_if(
            pSector->begin(),
            pSector->end(),
            [maxHeight](const RenIVertex& vertex) { return vertex.z >= maxHeight; });

        // It is quite possible that at the extremes of elevation there is no vertex
        // that satisfies the max height condition (especially with a lot of sectors).
        // We do not have to check for the minimum as the >= will always get a vertex
        // if there is one. The south cap rendering cuts out when this gets to low as well.
        if (itMax == pSector->end())
        {
            itMax = pSector->end();
            --itMax;
        }

        ASSERT_INFO(minHeight_);
        ASSERT(itMin != pSector->end(), "No minimum vertex could be found.");

        ptrdiff_t size = 0;

        // std::distance(itMin, itMax, size);
        size = std::distance(itMin, itMax);

        ASSERT(size > 0, "Tried to render an array of vertices with size == zero.");

        pVertexPtrs_->push_back(&(*itMin));
        pSizes_->push_back(size);
    }

private:
    ctl_vector<RenIVertex*>* const pVertexPtrs_;
    ctl_vector<int>* const pSizes_;
    float minHeight_;
    float maxHeight_;
};

#endif /* _INTERNAL_RENDER_STARS_IMPL_FUNCTORS_HPP ********/
