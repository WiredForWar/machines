/*
 * A V A I L O D S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#define _INSTANTIATE_TEMPLATE_CLASSES

#include "base/error.hpp"
#include "planeted/availods.hpp"

// static
PedAvailableTileLods& PedAvailableTileLods::instance()
{
    static PedAvailableTileLods instance_;
    return instance_;
}

PedAvailableTileLods::PedAvailableTileLods()
    : SysFileEnumerator(SysPathName(), SysPathName())
{
    lodIndex_ = 0;

    TEST_INVARIANT;

    POST(lodIndex_ == 0);
}

PedAvailableTileLods::~PedAvailableTileLods()
{
    TEST_INVARIANT;
}

void PedAvailableTileLods::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void PedAvailableTileLods::initialise(const SysPathName& lodDir)
{
    ASSERT_INFO(lodDir);

    fileSpecification("*.lod");
    directory(lodDir);

    find();

    // In fact LOD files are not strictly necessary (anymore)
    // POST(!isEmpty());
    // ALWAYS_ASSERT(!isEmpty(), ("No tile lods found for this planet: " + lodDir.pathname()).c_str())
}

const SysPathName& PedAvailableTileLods::next()
{
    ++lodIndex_;

    if (files().size() <= lodIndex_)
        lodIndex_ = 0;

    return files()[lodIndex_].pathName();
}

const SysPathName& PedAvailableTileLods::prev()
{
    if (lodIndex_ == 0)
    {
        lodIndex_ = files().size();
    }

    --lodIndex_;

    return files()[lodIndex_].pathName();
}

const SysPathName& PedAvailableTileLods::current()
{
    return files()[lodIndex_].pathName();
}

std::ostream& operator<<(std::ostream& o, const PedAvailableTileLods& t)
{

    o << "PedAvailableTileLods " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "PedAvailableTileLods " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End AVAILODS.CPP *************************************************/
