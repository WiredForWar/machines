/*
 * B N D I D G E N . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "utility/BoundedIdGenerator.hpp"

#ifndef _INLINE
#include "utility/BoundedIdGenerator.ipp"
#endif

#include "base/Diag.hpp"

#include <optional>

UtlBoundedIdGenerator::UtlBoundedIdGenerator(UtlId upperBound)
    : upperBound_(upperBound)
    , lastAllocatedId_(0)
    , nUnused_(upperBound)
    , aFlags_(nullptr)
{
    PRE(upperBound != 0);

    // Allocate an array of flags
    aFlags_ = _NEW_ARRAY(uchar, upperBound);

    // Initialise them
    for (UtlId i = upperBound; i--;)
        aFlags_[i] = 0;

    TEST_INVARIANT;
}

UtlBoundedIdGenerator::~UtlBoundedIdGenerator()
{
    TEST_INVARIANT;

    // Delete the flag array
    _DELETE_ARRAY(aFlags_);
}

std::optional<UtlId> UtlBoundedIdGenerator::nextId()
{
    // The scan is bounded by the size of the pool rather than by the remaining
    // count, so that a count which has drifted out of step with the flags
    // yields no id instead of looping for ever.
    for (UtlId scanned = 0; scanned != upperBound_; ++scanned)
    {
        ++lastAllocatedId_;
        if (lastAllocatedId_ == upperBound_)
            lastAllocatedId_ = 0;

        if (aFlags_[lastAllocatedId_] == 0)
        {
            // Decrement remaining count
            --nUnused_;

            // Mark as in use
            aFlags_[lastAllocatedId_] = uchar(1);

            return lastAllocatedId_;
        }
    }

    return std::nullopt;
}

std::optional<UtlId> UtlBoundedIdGenerator::nextId(UtlId minId, UtlId maxId)
{
    PRE(maxId > minId);
    PRE(maxId <= upperBound_);

    // bounds check the lastAllocatedId_ - this could mean that for each Id we start again at minId
    // but if the client code is correct then this shouldn't happen.
    if (lastAllocatedId_ < minId)
        lastAllocatedId_ = minId;

    for (UtlId scanned = 0; scanned != maxId - minId; ++scanned)
    {
        ++lastAllocatedId_;
        if (lastAllocatedId_ >= maxId)
            lastAllocatedId_ = minId;

        if (aFlags_[lastAllocatedId_] == 0)
        {
            // Decrement remaining count
            --nUnused_;

            // Mark as in use
            aFlags_[lastAllocatedId_] = uchar(1);

            return lastAllocatedId_;
        }
    }

    return std::nullopt;
}

void UtlBoundedIdGenerator::allocateId(UtlId id)
{
    PRE(! isAllocated(id));

    // Decrement remaining count
    --nUnused_;

    // Mark as in use
    aFlags_[id] = uchar(1);
}

void UtlBoundedIdGenerator::freeId(UtlId id)
{
    PRE(isAllocated(id));

    // Reset the flag and increment the free count
    aFlags_[id] = 0;
    ++nUnused_;
}

void UtlBoundedIdGenerator::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const UtlBoundedIdGenerator& t)
{

    o << "UtlBoundedIdGenerator " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "UtlBoundedIdGenerator " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End BNDIDGEN.CPP *************************************************/
