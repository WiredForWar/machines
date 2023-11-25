/*
 * T E X S E T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "render/internal/texseti.hpp"
#include "render/texset.hpp"

RenTextureSet::RenTextureSet()
    : pImpl_(new RenTextureSetImpl())
{
    TEST_INVARIANT;
    POST(! isLoaded());
}

RenTextureSet::RenTextureSet(const SysPathName& pathName)
    : pImpl_(new RenTextureSetImpl(pathName))
{
    TEST_INVARIANT;
    POST(isLoaded());
}

RenTextureSet::RenTextureSet(const SysPathName& pathName, IProgressReporter* pReporter)
    : pImpl_(new RenTextureSetImpl(pathName, pReporter))
{
    TEST_INVARIANT;
    POST(isLoaded());
}

RenTextureSet::~RenTextureSet()
{
    TEST_INVARIANT;
    delete pImpl_;
}

void RenTextureSet::load(const SysPathName& pathName)
{
    PRE(! isLoaded());
    pImpl_->load(pathName);
}

void RenTextureSet::load(const SysPathName& pathName, IProgressReporter* pReporter)
{
    PRE(! isLoaded());
    PRE(pReporter);

    pImpl_->load(pathName, pReporter);
}

bool RenTextureSet::isLoaded() const
{
    return pImpl_->isLoaded_;
}

void RenTextureSet::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const RenTextureSet& t)
{

    o << "RenTextureSet " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "RenTextureSet " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End TEXSET.CPP ***************************************************/
