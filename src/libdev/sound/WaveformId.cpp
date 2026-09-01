/*
 * S N D W A V I D . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "sound/WaveformId.hpp"
#include "system/PathName.hpp"

class SndWaveformIdImpl
{
public:
    SndWaveformIdImpl();
    SndWaveformIdImpl(const SysPathName& pathname);
    ~SndWaveformIdImpl();

    SysPathName pathname_;
};

SndWaveformIdImpl::SndWaveformIdImpl()
{
    // Empty
}

SndWaveformIdImpl::SndWaveformIdImpl(const SysPathName& pathname)
    : pathname_(pathname)
{
    PRE_INFO(pathname);
    PRE(pathname.existsAsFile());
}

SndWaveformIdImpl::~SndWaveformIdImpl()
{
    // Empty
}

/////////////////////////////////////////////////////////////

SndWaveformId::SndWaveformId()
{
    ASSERT(false, "This constructor should never be called");
}

SndWaveformId::SndWaveformId(const SndWaveformId& newId)
    : pImpl_(new SndWaveformIdImpl(*newId.pImpl_))
{
}

SndWaveformId::~SndWaveformId()
{
    delete pImpl_;
}

SndWaveformId::SndWaveformId(const SysPathName& pathname)
    : pImpl_(new SndWaveformIdImpl(pathname))
{
    // Empty
}

const SysPathName& SndWaveformId::pathname() const
{
    CB_DEPIMPL(SysPathName, pathname_);

    return pathname_;
}

SndWaveformId& SndWaveformId::operator=(const SndWaveformId& rhs)
{
    CB_DEPIMPL(SysPathName, pathname_);

    pathname_ = rhs.pImpl_->pathname_;
    return (*this);
}

bool operator==(const SndWaveformId& lhs, const SndWaveformId& rhs)
{
    return (lhs.pImpl_->pathname_ == rhs.pImpl_->pathname_);
}

bool operator<(const SndWaveformId& lhs, const SndWaveformId& rhs)
{
    return (lhs.pathname() < rhs.pathname());
}
