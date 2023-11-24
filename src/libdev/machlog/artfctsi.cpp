/*
 * A R T F C T S I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "mathex/point2d.hpp"
#include "machlog/internal/artfctsi.hpp"
#include "ctl/vector.hpp"
#include "world4d/genrepos.hpp"
#include "system/pathname.hpp"
#include "machlog/internal/artsdata.hpp"

PER_DEFINE_PERSISTENT(MachLogArtefactsImpl);

MachLogArtefactsImpl::MachLogArtefactsImpl()
    : pExemplars_(new W4dGenericRepository)
    , pData_(new(MachLogArtefactsData))
    , artefactsParsed_(false)
    , pArtefactPathName_(nullptr)
{

    TEST_INVARIANT;
}

MachLogArtefactsImpl::~MachLogArtefactsImpl()
{
    TEST_INVARIANT;
    delete pExemplars_;
    delete pData_;
}

void MachLogArtefactsImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogArtefactsImpl& t)
{

    o << "MachLogArtefactsImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogArtefactsImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogArtefactsImpl& artefactsImpl)
{
    ostr << artefactsImpl.pArtefactPathName_;
}

void perRead(PerIstream& istr, MachLogArtefactsImpl& artefactsImpl)
{
    istr >> artefactsImpl.pArtefactPathName_;
}

/* End ARTFCTSI.CPP *************************************************/
