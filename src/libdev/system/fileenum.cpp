/*
 * F I L E E N U M . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "system/fileenum.hpp"

SysFileEnumerator::SysFileEnumerator(const SysPathName& directory, const SysPathName& fileSpecification)
    : SysFileFinder(directory, fileSpecification)
{
    files_.reserve(512);
}

void SysFileEnumerator::CLASS_INVARIANT
{
}

const SysFileEnumerator::FileDatas& SysFileEnumerator::files() const
{
    return files_;
}

void SysFileEnumerator::clearFiles()
{
    files_.clear();
}

bool SysFileEnumerator::isEmpty() const
{
    return files_.empty();
}

// virtual
SysFileFinder::ResultType SysFileEnumerator::processFile(const SysFileData& fileData)
{
    files_.push_back(fileData);
    return SysFileFinder::CONTINUE;
}

std::ostream& operator<<(std::ostream& o, const SysFileEnumerator& t)
{

    o << "SysFileEnumerator " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "SysFileEnumerator " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End FILEENUM.CPP *************************************************/
