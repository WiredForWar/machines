/*
 * F I L T E R . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <sstream>

#include "base/private/filter.hpp"
#include "base/internal/filtbuff.hpp"

BaseFilterOstream::BaseFilterOstream(std::ostream& ostr)
    : std::ostream(pFilterBuffer())
    , pOstr_(&ostr)
{
}

BaseFilterOstream::BaseFilterOstream()
    : std::ostream(pFilterBuffer())
    , pOstr_(nullptr)
{
}

BaseFilterOstream::~BaseFilterOstream()
{

    delete pBuffer_;
}

void BaseFilterOstream::stream(std::ostream& ostr)
{
    pOstr_ = &ostr;
}

BaseFilterBuffer* BaseFilterOstream::pFilterBuffer()
{
    pBuffer_ = new BaseFilterBuffer(this);
    return pBuffer_;
}

void BaseFilterOstream::filterCharacters(const char* pBuf, size_t nChars)
{
    doFilterCharacters(pBuf, nChars);
}

void BaseFilterOstream::outputCharacters(const char* pBuf, size_t nChars)
{
    PRE(pOstr_ != nullptr);

    pOstr_->write(pBuf, nChars);
}

/* End FILTER.CPP *************************************************/
