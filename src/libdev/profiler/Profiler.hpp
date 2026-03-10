/*
 * P R O F I L E R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    ProProfiler

    Provides stack tracing and output stream facilities for diagnostics.
*/

#ifndef _PROFILER_PROFILER_HPP
#define _PROFILER_PROFILER_HPP

#include "base/base.hpp"

#include <fstream>

class BaseLogBuffer;

class ProProfiler
// Canonical form revoked
{
public:
    //  Singleton class
    static ProProfiler& instance();
    ~ProProfiler();

    std::ostream& outputStream();
    void closeOutputStream();

    // Prints the current call stack in standard form to outStream.
    // If doTraceAnchor is true, the name of the anchor function and its address is prepended.
    // The line number is added. (Zero implies not known).
    // If extraString is non empty, it is added.
    void traceStack(std::ostream& outStream, bool doTraceAnchor, uint32_t lineNumber, const char* extraString);

    // Export the address of the call stack
    const size_t* pCallStack() const;

    // Current number of entries in the call stack
    size_t nCallStackEntries() const;

    // Trace the supplied call stack pCallStack with nStackFrames frames to outStream
    void traceStack(
        std::ostream& outStream,
        bool doTraceAnchor,
        size_t nStackFrames,
        const size_t* pCallStack,
        uint32_t lineNumber,
        const char* extraString);

    // Enable/disable/test whether all calls to traceStack are written to an internal buffer instead
    // of designated stream
    void isBufferingOutput(bool is);
    bool isBufferingOutput() const;

    // Clear the internal buffer
    void clearBuffer();
    // PRE( isBufferingOutput() );

    // Flush the internal buffer to outStream
    void writeBuffer(std::ostream& outStream);
    // PRE( isBufferingOutput() );

    // Set to induce a crash on a call to print the stack when profiling.
    // Used to help trap hanging bugs.
    void crashOnPrint(bool crash);
    bool crashOnPrint();

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const ProProfiler& t);

private:
    ProProfiler(const ProProfiler&);
    ProProfiler& operator=(const ProProfiler&);
    bool operator==(const ProProfiler&);

    ProProfiler();

    std::ofstream outputStream_;

    bool isBufferingOutput_{};
    BaseLogBuffer* pMemoryBuffer_{};
    bool crashOnPrint_{};
};

#endif

/* End PROFILER.HPP *************************************************/
