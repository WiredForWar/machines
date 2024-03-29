/*
 * M E T A I S T R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "system/metaistr.hpp"
#include "system/metafile.hpp"
#include "system/pathname.hpp"

SysMetaFileIstream::SysMetaFileIstream(
    const SysMetaFile& metaFile,
    const SysPathName& fileName,
    std::ios::openmode openMode)
{
    PRE(metaFile.hasFile(fileName));
}

SysMetaFileIstream::~SysMetaFileIstream()
{
}

void SysMetaFileIstream::CLASS_INVARIANT
{
}

std::ostream& operator<<(std::ostream& o, const SysMetaFileIstream& t)
{

    o << "SysMetaFileIstream " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "SysMetaFileIstream " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End METAISTR.CPP *************************************************/
