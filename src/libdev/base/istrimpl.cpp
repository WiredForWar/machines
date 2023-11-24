/*
 * I S T R I M P L . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <iostream>

#include "base/internal/istrimpl.hpp"
#include "base/internal/istrbuff.hpp"
#include "base/IProgressReporter.hpp"

PerIstreamImpl::PerIstreamImpl(PerIstream* pStream)
    : pBuffer_(new PerIstreamBuffer(pStream))
    , pReporter_(nullptr)
    , fileSize_(0)
    , bytesRead_(0)
    , callBackAfter_(0)
{

    TEST_INVARIANT;
}

PerIstreamImpl::PerIstreamImpl(PerIstream* pStream, std::istream& istr, IProgressReporter* pReporter)
    : pBuffer_(new PerIstreamBuffer(pStream))
    , pReporter_(pReporter)
    , fileSize_(0)
    , bytesRead_(0)
    , callBackAfter_(0)
{
    PRE(pReporter != nullptr);

    //  Work out the length of the input file
    std::streampos originalPosition = istr.tellg();
    istr.seekg(0, std::ios::end);
    std::streampos endPosition = istr.tellg();
    istr.seekg(originalPosition);

    fileSize_ = endPosition - originalPosition;
}

PerIstreamImpl::~PerIstreamImpl()
{
    TEST_INVARIANT;

    delete pBuffer_;
}

void PerIstreamImpl::logDataRead(size_t bytesRead)
{
    if (pReporter_)
    {
        bytesRead_ += bytesRead;

        if (bytesRead_ >= callBackAfter_)
            callBackAfter_ = bytesRead_ + pReporter_->report(bytesRead_, fileSize_);
    }
}

void PerIstreamImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const PerIstreamImpl& t)
{

    o << "PerIstreamImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "PerIstreamImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End ISTRIMPL.CPP *************************************************/
