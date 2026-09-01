/*
 * D I A G I N T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "base/private/DiagInternal.hpp"
#include "base/internal/DiagStreams.hpp"
#include "base/Tracker.hpp"

//  An exception to our normal "no globals" rule. These are only plain data types
//  so won't give us any memory allocation problems. They are needed to allow
//  various diagnostics to be turned on and off efficiently.

bool DiagInternal::trackAllObjects_ = false;
bool DiagInternal::checkCountedPointers_ = false;
bool DiagInternal::checkInvariants_ = false;

// static
bool DiagInternal::streamEnabled(DiagStreamType type)
{
    PRE(type < DiagStreams::instance().nStreams());

    return DiagStreams::instance().diagStreamEnabled_[type];
}

// static
void DiagInternal::enableStream(DiagStreamType type)
{
    PRE(type < DiagStreams::instance().nStreams());

    if (DiagStreams::instance().diagStreams_[type].hasDestination())
        DiagStreams::instance().diagStreamEnabled_[type] = true;
}

// static
void DiagInternal::disableStream(DiagStreamType type)
{
    PRE(type < DiagStreams::instance().nStreams());

    DiagStreams::instance().diagStreamEnabled_[type] = false;
}

// static
DiagPointerTracker& DiagInternal::countedPointerTracker()
{
    static DiagPointerTracker countedPointerTracker_;

    return countedPointerTracker_;
}

/* End DIAGINT.CPP **************************************************/
