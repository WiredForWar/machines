/*
 * P R O F I L E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "profiler/Profiler.hpp"
#include "base/LogBuffer.hpp"

static size_t count;
static size_t call_stack;

// static
ProProfiler& ProProfiler::instance()
{
    static ProProfiler instance_;
    return instance_;
}

ProProfiler::ProProfiler()
    : outputStream_("profiler.dat")
{
}

ProProfiler::~ProProfiler()
{
    TEST_INVARIANT;
    delete pMemoryBuffer_;
}

std::ostream& ProProfiler::outputStream()
{
    return outputStream_;
}

void ProProfiler::closeOutputStream()
{
    outputStream_.close();
}

void ProProfiler::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const ProProfiler& t)
{
    o << "ProProfiler " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "ProProfiler " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void ProProfiler::traceStack(
    std::ostream& mystr,
    bool /*doTraceAnchor*/,
    size_t nStackFrames,
    const size_t* pCallStack,
    uint32_t lineNumber,
    const char* extraString)
{
    // This is necessary because of compiler rubbish
    std::ostream* pOstr = &mystr;
    if (isBufferingOutput_)
        pOstr = pMemoryBuffer_;
    std::ostream& ostr = *pOstr;

    // Do the left marker
    ostr << "[[[";

    // Now the actual call stack
    for (size_t i = 0; i < nStackFrames; ++i)
    {
        void* p = reinterpret_cast<void*>((*(pCallStack + i)));
        ostr << " " << p;
    }

    // And the line number
    ostr << " [" << lineNumber << "]";

    // And any extra string
    if (extraString && extraString[0] != '\0')
        ostr << " { " << extraString << " }";

    // And the terminator
    ostr << " ]]]" << std::endl;
}

void ProProfiler::isBufferingOutput(bool is)
{
    // Check which way
    if (is && ! isBufferingOutput_)
    {
        // starting buffering
        isBufferingOutput_ = true;
        pMemoryBuffer_ = new BaseLogBuffer(256000);
    }
    else if (! is && isBufferingOutput_)
    {
        // stopping buffering
        isBufferingOutput_ = false;
        delete pMemoryBuffer_;
        pMemoryBuffer_ = nullptr;
    }
}

bool ProProfiler::isBufferingOutput() const
{
    return isBufferingOutput_;
}

void ProProfiler::clearBuffer()
{
    PRE(isBufferingOutput());
    pMemoryBuffer_->clear();
}

void ProProfiler::writeBuffer(std::ostream& outStream)
{
    PRE(isBufferingOutput());
    outStream << *pMemoryBuffer_;
    pMemoryBuffer_->clear();
}

void ProProfiler::crashOnPrint(bool crash)
{
    crashOnPrint_ = crash;
}

bool ProProfiler::crashOnPrint()
{
    return crashOnPrint_;
}

const size_t* ProProfiler::pCallStack() const
{
    return &call_stack;
}

size_t ProProfiler::nCallStackEntries() const
{
    return count;
}

void ProProfiler::traceStack(std::ostream& outStream, bool doTraceAnchor, uint32_t lineNumber, const char* extraString)
{
    traceStack(outStream, doTraceAnchor, count, &call_stack, lineNumber, extraString);
}

/* End PROFILER.CPP *************************************************/
