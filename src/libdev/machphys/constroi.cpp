/*
 * C O N S T R O I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/internal/constroi.hpp"

#include "base/diag.hpp"
#include "world4d/domain.hpp"
#include "world4d/genecomp.hpp"
#include "world4d/link.hpp"
#include "machphys/damage.hpp"
#include "profiler/profiler.hpp"

PER_DEFINE_PERSISTENT(MachPhysConstructionImpl);

MachPhysConstructionImpl::MachPhysConstructionImpl(
    double height,
    size_t level,
    MachPhys::Race race,
    const MachPhysConstructionData& data,
    const MexTransform3d& globalTransform)
    : height_(height)
    , level_(level)
    , isWorking_(false)
    , pInteriorDomain_(nullptr)
    , pInteriorComposite_(nullptr)
    , constructionData_(data, globalTransform)
    , pDamage_(nullptr)
    , percentageComplete_(0)
    , race_(race)
    , pSavedPadsTransforms_(nullptr)
    , completionBand_(BAND0)
    , completionVisualised_(false)
{

    TEST_INVARIANT;
}

MachPhysConstructionImpl::~MachPhysConstructionImpl()
{
    TEST_INVARIANT;
}

void MachPhysConstructionImpl::createSavedPadsTransforms()
{
    PRE(pSavedPadsTransforms_ == nullptr);

    pSavedPadsTransforms_ = new LinkTransforms;
    pSavedPadsTransforms_->reserve(15);
}

MachPhysConstructionImpl::LinkTransforms& MachPhysConstructionImpl::savedPadsTransforms()
{
    PRE(pSavedPadsTransforms_ != nullptr);

    return *pSavedPadsTransforms_;
}

const MachPhysConstructionImpl::LinkTransforms& MachPhysConstructionImpl::savedPadsTransforms() const
{
    PRE(pSavedPadsTransforms_ != nullptr);

    return *pSavedPadsTransforms_;
}

// If id is found in the saved transforms, the function returns successfully and the transform is returned
bool MachPhysConstructionImpl::savedPadTransform(W4dLink::Id id, MexTransform3d* padTransform) const
{
    PRE(padTransform);

    bool result = false;
    LinkTransforms::const_iterator it;
    for (it = savedPadsTransforms().begin(); it != savedPadsTransforms().end() && ! result; ++it)
    {
        if ((*it).id == id)
        {
            LIONEL_STREAM("(*it).id " << (*it).id << std::endl);
            result = true;
            *padTransform = (*it).transform;
        }
    }
    return result;
}

void MachPhysConstructionImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysConstructionImpl& t)
{

    o << "MachPhysConstructionImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysConstructionImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysConstructionImpl& construction)
{
    ostr << construction.adornments_;
    ostr << construction.percentageComplete_;
    ostr << construction.height_;
    ostr << construction.level_;
    ostr << construction.race_;
    ostr << construction.isWorking_;
    ostr << construction.pInteriorDomain_;
    ostr << construction.pInteriorComposite_;
    ostr << construction.pDamage_;
}

void perRead(PerIstream& istr, MachPhysConstructionImpl& construction)
{
    istr >> construction.adornments_;
    istr >> construction.percentageComplete_;
    istr >> construction.height_;
    istr >> construction.level_;
    istr >> construction.race_;
    istr >> construction.isWorking_;
    istr >> construction.pInteriorDomain_;
    istr >> construction.pInteriorComposite_;
    istr >> construction.pDamage_;
}

MachPhysConstructionImpl::MachPhysConstructionImpl(PerConstructor)
    : isWorking_(false)
    , pInteriorDomain_(nullptr)
    , pInteriorComposite_(nullptr)
    , pDamage_(nullptr)
    , percentageComplete_(0)
    , pSavedPadsTransforms_(nullptr)
    , completionBand_(BAND0)
    , completionVisualised_(false)
{
}

/* End CONSTROI.CPP *************************************************/
