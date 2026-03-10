/***********************************************************

  S T A R S I . C P P
  (c) Charybdis Limited, 1998. All Rights Reserved

***********************************************************/

#include "render/render.hpp"
#include "render/Colour.hpp"
#include "render/Camera.hpp"
#include "render/Stats.hpp"
#include "render/Material.hpp"
#include "render/internal/StarsImpl.hpp"
#include "render/internal/FnStars.hpp"
#include "render/internal/DeviceImpl.hpp"
#include "render/Device.hpp"
#include "render/internal/BackendCommands.hpp"
#include "render/internal/ColourPack.hpp"

#include "ctl/Algorithm.hpp"

#include "world4d/Manager.hpp"

#include <algorithm>
#include <cmath>

#include "mathex/Angle.hpp"
#include "mathex/EulerAngles.hpp"
#include "mathex/Transform3d.hpp"

#include "utility/Percentage.hpp"

#ifndef _INLINE
#include "render/internal/StarsImpl.ipp"
#endif

// Attempt to match the Planck blackbody locus: maps a stellar temperature
// in Kelvin to an RGB triplet normalised so the brightest channel is 1.0.
// Based on Tanner Helland's approximation of the CIE 1964 colour-matching
// data, tweaked for visual punch rather than scientific accuracy.
static void starTemperatureToRGB(float kelvin, float& r, float& g, float& b)
{
    float temp = kelvin / 100.0f;

    // Red channel
    if (temp <= 66.0f)
    {
        r = 1.0f;
    }
    else
    {
        float x = temp - 60.0f;
        r = 1.292936f * std::pow(x, -0.1332047f);
    }

    // Green channel
    if (temp <= 66.0f)
    {
        g = 0.3900816f * std::log(temp) - 0.6318414f;
    }
    else
    {
        float x = temp - 60.0f;
        g = 1.129891f * std::pow(x, -0.0755148f);
    }

    // Blue channel
    if (temp >= 66.0f)
    {
        b = 1.0f;
    }
    else if (temp <= 19.0f)
    {
        b = 0.0f;
    }
    else
    {
        float x = temp - 10.0f;
        b = 0.5432068f * std::log(x) - 1.1962541f;
    }

    r = std::clamp(r, 0.0f, 1.0f);
    g = std::clamp(g, 0.0f, 1.0f);
    b = std::clamp(b, 0.0f, 1.0f);

    // Normalise so the brightest channel is 1.0 — preserves hue saturation.
    float peak = std::max({ r, g, b });
    if (peak > 0.0f)
    {
        r /= peak;
        g /= peak;
        b /= peak;
    }
}

// Sample a random star temperature from a distribution inspired by real
// spectral-type abundances. The night sky is dominated by K and M dwarfs
// (cool/orange-red) with fewer hot O/B stars, but the brightest naked-eye
// stars include many A/F types. We bias slightly toward variety so the
// sky looks interesting.
//   O  (~40000 K) :  1%     B  (~20000 K) :  3%
//   A  (~9000 K)  : 10%     F  (~7000 K)  : 15%
//   G  (~5800 K)  : 20%     K  (~4500 K)  : 30%
//   M  (~3200 K)  : 18%     Red giant (~3000 K, rare bright) : 3%
static float randomStarTemperature(MexBasicRandom* rng)
{
    float roll = mexRandomScalar(rng, 0.0f, 1.0f);

    if (roll < 0.03f)          // Red giant / deep red
        return mexRandomScalar(rng, 2500.0f, 3200.0f);
    if (roll < 0.21f)          // M dwarf
        return mexRandomScalar(rng, 3200.0f, 3900.0f);
    if (roll < 0.51f)          // K
        return mexRandomScalar(rng, 3900.0f, 5300.0f);
    if (roll < 0.71f)          // G (Sun-like)
        return mexRandomScalar(rng, 5300.0f, 6000.0f);
    if (roll < 0.86f)          // F
        return mexRandomScalar(rng, 6000.0f, 7500.0f);
    if (roll < 0.96f)          // A
        return mexRandomScalar(rng, 7500.0f, 10000.0f);
    if (roll < 0.99f)          // B
        return mexRandomScalar(rng, 10000.0f, 28000.0f);
    // O
    return mexRandomScalar(rng, 28000.0f, 40000.0f);
}

static const int N_SECTORS = 16;
static const MATHEX_SCALAR SECTOR_WIDTH = 2 * Mathex::PI / N_SECTORS;
static const RenColour STAR_COLOUR = RenColour::white();
// Because of the non-cartesian culling a slight fudge factor is needed for the illusion.
static const MexRadians FUDGE_FACTOR = Mathex::PI / 24;
static glm::mat4 starsXform((1), 0.0, 0.0, 0.0, 0.0, (1), 0.0, 0.0, 0.0, 0.0, (1), 0.0, 0.0, 0.0, 0.0, (1));

RenIStarsImpl::RenIStarsImpl(RenStars::Configuration config, MATHEX_SCALAR radius, uint nStars)
    : configuration_(config)
    , nStars_(nStars)
    , radius_(radius)
    , sectors_(N_SECTORS)
{
    PRE(radius_ >= 1.0);
    PRE(nStars_ > 0);
    PRE(sectors_.size() == N_SECTORS);

    // Make sure that the member vectors of sectors are reserved.
    // TODO is this renIStarsImplReserveVectorsOp implemented anywhere?
    // ctl_for_each(sectors_, renIStarsImplReserveVectorsOp);

    RenIStarsImplVertexGen* pVertexGen = nullptr;
    if (configuration() == RenStars::SPHERICAL)
        pVertexGen = new RenIStarsImplSphericalVertexGen(radius_, STAR_COLOUR);
    else if (configuration() == RenStars::HEMISPHERICAL)
        pVertexGen = new RenIStarsImplHemisphericalVertexGen(radius_, STAR_COLOUR);
    else
        ASSERT_FAIL("An invalid stars configuration was found in RenIStarsImpl ctor.");

    ASSERT(pVertexGen, "The allocation of the vertex generator failed.");

    RenIStarsImplVertexInserterOp vertexInserter(&sectors_, N_SECTORS, SECTOR_WIDTH);

    // Create a set of random vertices in a particular configuration.
    for (int index = 0; index < nStars; ++index)
        vertexInserter((*pVertexGen)());

    // Sort the vertices by height. The lowest at sectors_[0].
    for (ctl_vector<RenIVertex>& sector : sectors_)
    {
        std::sort(sector.begin(), sector.end(), RenIStarsImplVertexHeightLesserOrEqual);
    }

    // Give the vertices some random alpha values.
    ctl_for_each(sectors_, RenIStarsImplRandomiseAlphasOp(0.3, 1.0));

    // Build per-star twinkle parameters (parallel to sorted sectors).
    MexBasicRandom twinkleRng = MexBasicRandom::constructSeededFromTime();
    twinkleSectors_.resize(N_SECTORS);
    for (int s = 0; s < N_SECTORS; ++s)
    {
        const size_t count = sectors_[s].size();
        twinkleSectors_[s].resize(count);
        for (size_t i = 0; i < count; ++i)
        {
            float alpha = static_cast<float>((sectors_[s][i].color >> 24) & 0xff) / 255.0f;
            twinkleSectors_[s][i].baseAlpha = alpha;
            twinkleSectors_[s][i].phase = mexRandomScalar(&twinkleRng, 0.0f, Mathex::PI_2);
            twinkleSectors_[s][i].frequency = mexRandomScalar(&twinkleRng, 0.5f, 2.5f);
            // Faint stars twinkle more noticeably, bright stars are steadier.
            twinkleSectors_[s][i].amplitude = 0.15f + 0.30f * (1.0f - alpha);

            // Assign stellar colour from a blackbody temperature curve.
            float kelvin = randomStarTemperature(&twinkleRng);
            starTemperatureToRGB(
                kelvin,
                twinkleSectors_[s][i].baseR,
                twinkleSectors_[s][i].baseG,
                twinkleSectors_[s][i].baseB);
        }
    }
    TEST_INVARIANT;
}

void RenIStarsImpl::render(
    UtlPercentage,
    const MexTransform3d& cameraXform,
    MexRadians verticalFOV,
    MexRadians horizontalFOV)
{
    // The assumption is not valid, horizontal POV can be greater than 90 degrees.
    // PRE(horizontalFOV.asScalar() <= Mathex::PI_DIV_2);
    RenIDeviceImpl* devImpl = RenIDeviceImpl::currentPimpl();

    ASSERT(devImpl, "There is no current device.");
    ASSERT(devImpl->currentCamera(), "There is no current camera.");

    RenColour currentColourFilter = devImpl->currentCamera()->colourFilter();

    // Animate star alpha (twinkling) and apply per-star colour * camera filter.
    updateTwinkle(currentColourFilter);

    // Pointers to vertex arrays for rendering.
    ctl_vector<RenIVertex*> vertexPtrs;
    vertexPtrs.reserve(5);
    // The sizes of the arrays pointed to by the above.
    ctl_vector<int> sizes;
    sizes.reserve(5);

    RenderRegion renRegion = renderRegion(cameraXform.rotationAsEulerAngles().elevation(), verticalFOV);

    switch (renRegion)
    {
        case NORTH_CAP:
            renderNorthCap(verticalFOV, &vertexPtrs, &sizes);
            break;

        case SOUTH_CAP:
            renderSouthCap(verticalFOV, &vertexPtrs, &sizes);
            break;

        case TROPICS:
            renderTropics(cameraXform, verticalFOV, horizontalFOV, &vertexPtrs, &sizes);
            break;

        case NONE:
            break;

        default:
            ASSERT_FAIL("An invalid render region was received in RenIStarsImpl");
            break;
    }

    // The number of vertices rendered.
    int nPoints = std::accumulate(sizes.begin(), sizes.end(), 0);
    // Alter the render statistics accordingly.
    RenStats* pStats = devImpl->statistics();
    if (pStats)
        pStats->incrPointsCount(nPoints);

    // Texture needs to be turned off.  Use a default untextured material.
    static RenMaterial defaultMat;
    devImpl->setMaterialHandles(defaultMat);

    RenDevice::current()->setModelMatrix(starsXform);

    // Stars use pre-colored vertices; disable GPU lighting.
    devImpl->clearGpuLightingState();

    // for_each(sectors_.begin(), sectors_.end(), RenIPrintSectorOp(Diag::instance().renderStream()));

    using BlendFactor = Ren::BackendBlendFactor;
    RenDevice::current()->recordCommand(
        Ren::Command::setBlendStateEnabled(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha));

    // Render the vertices.
    static const RenMaterial emptyMat;
    ASSERT_INFO(vertexPtrs.size());
    ASSERT_INFO(sizes.size());
    ASSERT(vertexPtrs.size() == sizes.size(), "Vertices count calculation went wrong");
    for (std::size_t i = 0; i < vertexPtrs.size(); ++i)
    {
        RenDevice::current()->renderPrimitive(vertexPtrs[i], sizes[i], emptyMat, Ren::PrimitiveTopology::Points);
    }

    TEST_INVARIANT;
}

// Private.
MATHEX_SCALAR RenIStarsImpl::absCapHeight(MexRadians verticalFOV) const
{
    return radius() * sin(Mathex::PI_DIV_2 - verticalFOV.asScalar());
}

// Private.
RenIStarsImpl::RenderRegion RenIStarsImpl::renderRegion(MexRadians elevation, MexRadians verticalFOV) const
{
    RenderRegion result = TROPICS;

    MATHEX_SCALAR lower = 0.0;
    MATHEX_SCALAR upper = 0.0;

    switch (configuration())
    {
        case RenStars::SPHERICAL:
            lower = verticalFOV.asScalar() / 2 - Mathex::PI_DIV_2;
            upper = Mathex::PI_DIV_2 - verticalFOV.asScalar() / 2;

            if (elevation.asScalar() < lower)
                result = NORTH_CAP;
            else if (elevation.asScalar() > upper)
                result = SOUTH_CAP;
            break;

        case RenStars::HEMISPHERICAL:
            lower = verticalFOV.asScalar() / 2 - Mathex::PI_DIV_2;
            upper = verticalFOV.asScalar() / 2;

            if (elevation.asScalar() < lower)
                result = NORTH_CAP;
            else if (elevation.asScalar() > upper)
                result = NONE;
            break;

        default:
            ASSERT_FAIL("An Invalid star field configuration was found in RenIStarsImpl.");
            break;
    }

    return result;
}

// Private.
void RenIStarsImpl::renderNorthCap(
    MexRadians verticalFOV,
    ctl_vector<RenIVertex*>* const pVertexPtrs,
    ctl_vector<int>* const pSizes)
{
    PRE(pVertexPtrs->empty());
    PRE(pSizes->empty());

    MATHEX_SCALAR rimHeight = absCapHeight(verticalFOV);

    ctl_for_each(sectors_, RenIStarsImplDefineNorthCapOp(rimHeight, pVertexPtrs, pSizes));
}

// Private.
void RenIStarsImpl::renderSouthCap(
    MexRadians verticalFOV,
    ctl_vector<RenIVertex*>* const pVertexPtrs,
    ctl_vector<int>* const pSizes)
{
    PRE(configuration() == RenStars::SPHERICAL);
    PRE(pVertexPtrs->empty());
    PRE(pSizes->empty());

    MATHEX_SCALAR rimHeight = -absCapHeight(verticalFOV);

    ctl_for_each(sectors_, RenIStarsImplDefineSouthCapOp(rimHeight, pVertexPtrs, pSizes));
}

// Private.
void RenIStarsImpl::renderTropics(
    const MexTransform3d& cameraXform,
    MexRadians verticalFOV,
    MexRadians horizontalFOV,
    ctl_vector<RenIVertex*>* const pVertexPtrs,
    ctl_vector<int>* const pSizes)
{
    PRE(pVertexPtrs->empty());
    PRE(pSizes->empty());

    // Perform a little test to see whether the caps need rendering.
    MexRadians absElevation = Mathex::abs(cameraXform.rotationAsEulerAngles().elevation().asScalar());

    // This tests that the renderRegion() code is doing it's thing.
    ASSERT(absElevation.asScalar() <= Mathex::PI_DIV_2 - verticalFOV.asScalar() / 2, "");
    ASSERT(horizontalFOV < MexDegrees(180), "The horizontal FOV is too large.");

    // This code linearly interpolates the elevation to the half angular width.
    // This is used to calculate how many sectors you can see at any time.
    // The outputs are between horizontalFOV/2 and 180 degrees. The translation
    // isn't strictly linear more like exponential but this does the trick.
    MATHEX_SCALAR halfAngularWidth = mexInterpolate(
        absElevation.asScalar(),
        0.0,
        Mathex::PI_DIV_2 - verticalFOV.asScalar() / 2,
        horizontalFOV.asScalar() / 2,
        Mathex::PI_DIV_2);

    ctl_vector<ctl_vector<RenIVertex>*> sectorPtrs;
    sectorPtrs.reserve(N_SECTORS / 4);

    cullSectors(cameraXform, halfAngularWidth, &sectorPtrs);

    MexRadians negatedElevation(-cameraXform.rotationAsEulerAngles().elevation().asScalar());
    float minHeight = radius() * sin(negatedElevation - verticalFOV / 2);
    float maxHeight = radius() * sin(negatedElevation + verticalFOV / 2);

    RenIStarsImplPegStarsOp pegStars(pVertexPtrs, pSizes, minHeight, maxHeight);

    ctl_for_each(sectorPtrs, pegStars);

    POST(pVertexPtrs->size() == pSizes->size());
}

// Private.
// The best place for optimisations I think.
void RenIStarsImpl::cullSectors(
    const MexTransform3d& cameraXform,
    MATHEX_SCALAR halfAngularWidth,
    ctl_vector<ctl_vector<RenIVertex>*>* const pSectorPtrs)
{
    PRE(halfAngularWidth > SECTOR_WIDTH / 2);
    PRE(pSectorPtrs);
    PRE(pSectorPtrs->empty());

    MATHEX_SCALAR anti = cameraXform.rotationAsEulerAngles().azimuth().asScalar() - halfAngularWidth;
    if (anti < -Mathex::PI)
        anti += 2 * Mathex::PI;

    MATHEX_SCALAR clock = cameraXform.rotationAsEulerAngles().azimuth().asScalar() + halfAngularWidth;
    if (clock > Mathex::PI)
        clock -= 2 * Mathex::PI;

    RenIStarsImplSectorClipBinPred sectorClip(anti, clock);

    MATHEX_SCALAR antiWall = -Mathex::PI;
    MATHEX_SCALAR clockWall = SECTOR_WIDTH - Mathex::PI;

    for (int sectorIndex = 0; sectorIndex < N_SECTORS;
         ++sectorIndex, antiWall += SECTOR_WIDTH, clockWall += SECTOR_WIDTH)
    {
        if (sectorClip(antiWall, clockWall))
            pSectorPtrs->push_back(&(sectors_[sectorIndex]));
    }
}

void RenIStarsImpl::updateTwinkle(const RenColour& colourFilter)
{
    float elapsed = static_cast<float>(W4dManager::instance().time());
    float filterR = mexClamp(colourFilter.r(), 0.0f, 1.0f);
    float filterG = mexClamp(colourFilter.g(), 0.0f, 1.0f);
    float filterB = mexClamp(colourFilter.b(), 0.0f, 1.0f);

    for (int s = 0; s < N_SECTORS; ++s)
    {
        ctl_vector<RenIVertex>& sector = sectors_[s];
        const std::vector<TwinkleParams>& twinkle = twinkleSectors_[s];
        for (size_t i = 0; i < sector.size(); ++i)
        {
            float amp = twinkle[i].amplitude;
            float modulated = twinkle[i].baseAlpha
                * ((1.0f - amp) + amp * std::sin(elapsed * twinkle[i].frequency + twinkle[i].phase));

            float r = twinkle[i].baseR * filterR;
            float g = twinkle[i].baseG * filterG;
            float b = twinkle[i].baseB * filterB;
            sector[i].color = packColour(r, g, b, modulated);
        }
    }
}

void RenIStarsImpl::CLASS_INVARIANT
{
    INVARIANT(this);
    INVARIANT(radius_ >= 1.0);
    INVARIANT(sectors_.size() == N_SECTORS);
}

/* End CULLEDPT.CPP ***************************************/
