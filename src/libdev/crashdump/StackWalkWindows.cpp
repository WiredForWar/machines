#include "crashdump/internal/StackWalk.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace CrashDump
{

namespace
{

// The unwinder is the one in ntdll rather than the one in dbghelp, because it
// needs no symbol handler to be initialised and takes no loader lock: a process
// that is dying may hold either. It reads the unwind tables the PE image
// already carries, which is what makes the frames of a stripped release binary
// recoverable at all.

void programCounterAndStack(const CONTEXT& context, DWORD64& programCounter, DWORD64& stackPointer)
{
#if defined(_M_ARM64) || defined(__aarch64__)
    programCounter = context.Pc;
    stackPointer = context.Sp;
#else
    programCounter = context.Rip;
    stackPointer = context.Rsp;
#endif
}

void setProgramCounterAndStack(CONTEXT& context, DWORD64 programCounter, DWORD64 stackPointer)
{
#if defined(_M_ARM64) || defined(__aarch64__)
    context.Pc = programCounter;
    context.Sp = stackPointer;
#else
    context.Rip = programCounter;
    context.Rsp = stackPointer;
#endif
}

void walk(StackTrace& trace, CONTEXT context, std::size_t skipFrames)
{
    std::size_t skipped{};
    bool firstFrame{ true };

    while (trace.frameCount < StackTrace::maxFrames)
    {
        DWORD64 programCounter{};
        DWORD64 stackPointer{};
        programCounterAndStack(context, programCounter, stackPointer);

        if (programCounter == 0)
        {
            return;
        }

        if (skipped < skipFrames)
        {
            ++skipped;
        }
        else
        {
            trace.frames[trace.frameCount] = static_cast<std::uintptr_t>(programCounter);
            ++trace.frameCount;
        }

        DWORD64 imageBase{};
        PRUNTIME_FUNCTION function = RtlLookupFunctionEntry(programCounter, &imageBase, nullptr);

        if (function == nullptr)
        {
            // A leaf function has no unwind entry, and its return address is
            // simply the top of the stack. That is only worth believing for the
            // innermost frame: anywhere else it means the tables have run out,
            // and following a guess would read whatever happened to be there.
            if (! firstFrame || stackPointer == 0)
            {
                return;
            }

            setProgramCounterAndStack(
                context,
                *reinterpret_cast<const DWORD64*>(stackPointer),
                stackPointer + sizeof(DWORD64));
        }
        else
        {
            PVOID handlerData{};
            DWORD64 establisherFrame{};

            RtlVirtualUnwind(
                UNW_FLAG_NHANDLER,
                imageBase,
                programCounter,
                function,
                &context,
                &handlerData,
                &establisherFrame,
                nullptr);
        }

        firstFrame = false;
    }
}

} // namespace

void captureStackTrace(StackTrace& trace, std::size_t skipFrames)
{
    CONTEXT context{};
    RtlCaptureContext(&context);

    // One extra for this function's own frame.
    walk(trace, context, skipFrames + 1);
}

void captureStackTraceFromContext(StackTrace& trace, void* context, void* thread)
{
    // The unwinder reads the stack of whichever thread the context describes,
    // and that stack stays in this process's address space either way, so the
    // handle is needed only to document the caller's obligation to have
    // suspended it.
    static_cast<void>(thread);

    if (context == nullptr)
    {
        return;
    }

    walk(trace, *static_cast<const CONTEXT*>(context), 0);
}

void warmUpStackWalk()
{
    // The unwind tables are part of the image and the unwinder is in ntdll, so
    // nothing here is resolved lazily and there is nothing to warm up.
}

} // namespace CrashDump
