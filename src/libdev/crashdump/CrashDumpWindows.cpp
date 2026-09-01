#include "crashdump/internal/CrashDumpInternal.hpp"
#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/StackWalk.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <dbghelp.h>

#include <atomic>
#include <exception>

#include <csignal>
#include <cstdint>
#include <cstdlib>

namespace CrashDump
{

namespace
{

LPTOP_LEVEL_EXCEPTION_FILTER previousFilter_{};

// The first thread to fail writes the report; any other that fails while it is
// doing so, and any failure the reporting itself provokes, is dropped rather
// than allowed to interleave or recurse.
std::atomic_flag reporting_ = ATOMIC_FLAG_INIT;

// A C++ exception reaches the filter as a Microsoft SEH exception carrying this
// code, spelling "msc" in its low bytes. There is no macro for it.
constexpr DWORD cppExceptionCode{ 0xe06d7363 };

const char* exceptionName(DWORD code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION:
            return "ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            return "ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:
            return "BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            return "DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            return "FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            return "FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT:
            return "FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION:
            return "FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW:
            return "FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK:
            return "FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW:
            return "FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            return "ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR:
            return "IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            return "INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:
            return "INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION:
            return "INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            return "NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION:
            return "PRIV_INSTRUCTION";
        case EXCEPTION_STACK_OVERFLOW:
            return "STACK_OVERFLOW";
        case STATUS_STACK_BUFFER_OVERRUN:
            return "STACK_BUFFER_OVERRUN";
        case cppExceptionCode:
            return "CPP_EXCEPTION";
        default:
            return "unknown";
    }
}

void writeMinidump(EXCEPTION_POINTERS* exception, DWORD faultingThreadId)
{
    NativeChar path[maxReportPath]{};

    if (! buildReportPath(path, maxReportPath, "crash", "dmp"))
    {
        return;
    }

    const HANDLE file
        = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file == INVALID_HANDLE_VALUE)
    {
        return;
    }

    MINIDUMP_EXCEPTION_INFORMATION information{};
    information.ThreadId = faultingThreadId;
    information.ExceptionPointers = exception;
    information.ClientPointers = FALSE;

    const MINIDUMP_TYPE type
        = static_cast<MINIDUMP_TYPE>(MiniDumpWithDataSegs | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);

    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        file,
        type,
        exception == nullptr ? nullptr : &information,
        nullptr,
        nullptr);

    CloseHandle(file);
}

// The companion to the minidump, so that a report from a player can be read
// without a debugger and symbolicated with nothing but the matching .debug file.
void writeTextReport(
    const char* cause,
    const char* detail,
    EXCEPTION_POINTERS* exception,
    const StackTrace* faultingStack)
{
    NativeChar path[maxReportPath]{};

    if (! buildReportPath(path, maxReportPath, "crash", "txt"))
    {
        return;
    }

    ReportWriter writer(path);

    writer.write("=== Machines crash report ===");
    writer.newLine();

    writer.write("Cause: ");
    writer.write(cause);

    if (detail != nullptr)
    {
        writer.write(" (");
        writer.write(detail);
        writer.write(")");
    }

    writer.newLine();

    if (exception != nullptr && exception->ExceptionRecord != nullptr)
    {
        const EXCEPTION_RECORD& record = *exception->ExceptionRecord;

        writer.write("Exception: ");
        writer.write(exceptionName(record.ExceptionCode));
        writer.write(" (");
        writer.writeAddress(record.ExceptionCode);
        writer.write(")");
        writer.newLine();

        writer.write("Faulting address: ");
        writer.writeAddress(reinterpret_cast<std::uintptr_t>(record.ExceptionAddress));
        writer.newLine();

        // An access violation says what it was doing and to what, and that pair
        // separates a null dereference from a write through a stale pointer
        // without any need for the minidump.
        if (record.ExceptionCode == EXCEPTION_ACCESS_VIOLATION && record.NumberParameters >= 2)
        {
            writer.write("Access: ");
            writer.write(record.ExceptionInformation[0] == 0 ? "read" : "write");
            writer.write(" of ");
            writer.writeAddress(static_cast<std::uintptr_t>(record.ExceptionInformation[1]));
            writer.newLine();
        }
    }

    writer.write("Process: ");
    writer.writeUnsigned(currentProcessId());
    writer.newLine();

    writeBuildInfo(writer);
    writeModuleInfo(writer);

    writer.newLine();
    writer.write("--- Call stack ---");
    writer.newLine();

    StackTrace trace;

    if (exception != nullptr && exception->ContextRecord != nullptr)
    {
        captureStackTraceFromContext(trace, exception->ContextRecord, GetCurrentThread());
    }
    else if (faultingStack != nullptr)
    {
        trace = *faultingStack;
    }

    writeStackTrace(writer, trace);

    writer.newLine();
    writer.write("=== End of report ===");
    writer.newLine();

    if (writer.isOpen())
    {
        announceReport(path);
    }
}

struct ReportRequest
{
    const char* cause{};
    const char* detail{};
    EXCEPTION_POINTERS* exception{};
    DWORD faultingThreadId{};
    const StackTrace* faultingStack{};
};

DWORD WINAPI writeReportOnOwnStack(LPVOID parameter)
{
    const ReportRequest& request = *static_cast<const ReportRequest*>(parameter);

    writeMinidump(request.exception, request.faultingThreadId);
    writeTextReport(request.cause, request.detail, request.exception, request.faultingStack);

    return 0;
}

void writeReport(const char* cause, const char* detail, EXCEPTION_POINTERS* exception)
{
    if (reporting_.test_and_set())
    {
        return;
    }

    StackTrace faultingStack;

    // Captured here, while still on the thread that is failing. A failure that
    // arrives without an exception context -- an abort, a terminate, a rejected
    // runtime argument -- has no context for the report thread to walk, and the
    // report thread's own stack says nothing whatever about what went wrong.
    if (exception == nullptr)
    {
        captureStackTrace(faultingStack, 1);
    }

    ReportRequest request{ cause, detail, exception, GetCurrentThreadId(), &faultingStack };

    // The report is written on a thread of its own, because the failure that
    // most needs one is a stack overflow, and a filter for that runs on the
    // stack that just ran out. Windows has already released the guard page by
    // then, so the kilobytes a path buffer and MiniDumpWriteDump want are not
    // there, and the attempt to write the report faults in its turn -- which is
    // how a stack overflow came to produce an access violation and no report at
    // all. A new thread gets a whole new stack.
    //
    // Reading the faulting thread's stack from here is safe: it is blocked in
    // the wait below, so nothing is moving, and both stacks are in this
    // process's address space.
    const HANDLE thread = CreateThread(nullptr, 0, writeReportOnOwnStack, &request, 0, nullptr);

    if (thread == nullptr)
    {
        // No thread to be had. Writing it here may fail, but failing to try
        // certainly produces nothing.
        writeReportOnOwnStack(&request);
        return;
    }

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
}

LONG WINAPI exceptionFilter(EXCEPTION_POINTERS* exception)
{
    writeReport("unhandled exception", nullptr, exception);

    // Terminating here rather than continuing the search keeps the failure
    // deterministic: passing it on invites the Windows Error Reporting dialog,
    // which waits for a person and so hangs an unattended run indefinitely.
    return EXCEPTION_EXECUTE_HANDLER;
}

// Everything below covers a way of dying that never reaches the filter above.

// An assertion calls abort(), and abort() raises SIGABRT without ever entering
// exception dispatch. Left alone, the most common failure in a debug build
// writes no report at all.
void abortHandler(int number)
{
    static_cast<void>(number);

    writeReport("abort", "raised by abort() -- usually a failed assertion", nullptr);

    // Returning would let the CRT call abort() again and re-enter this handler.
    std::_Exit(3);
}

// An exception that escapes main, a throw from a noexcept function, and a
// failed rethrow all end here rather than in the filter.
void terminateHandler()
{
    writeReport("terminate", "an exception was not handled", nullptr);

    std::_Exit(3);
}

void purecallHandler()
{
    writeReport("pure virtual call", "a virtual function was called during construction or destruction", nullptr);

    std::_Exit(3);
}

#ifdef HAVE_SET_INVALID_PARAMETER_HANDLER
// The CRT's own validation -- a bad iterator, a bad file handle, a bad format
// string. Without this hook it goes straight to __fastfail, which the kernel
// turns into an immediate STATUS_STACK_BUFFER_OVERRUN termination that no
// user-mode handler, vectored or otherwise, is given a chance to see. Exit
// code 0xC0000409 and an empty dump folder is what that looks like from
// outside.
void invalidParameterHandler(
    const wchar_t* expression,
    const wchar_t* function,
    const wchar_t* file,
    unsigned int line,
    uintptr_t reserved)
{
    static_cast<void>(expression);
    static_cast<void>(function);
    static_cast<void>(file);
    static_cast<void>(line);
    static_cast<void>(reserved);

    writeReport("invalid parameter", "the C runtime rejected an argument", nullptr);

    std::_Exit(3);
}
#endif

} // namespace

unsigned long long currentProcessId()
{
    return GetCurrentProcessId();
}

void writeModuleInfo(ReportWriter& writer)
{
    // Every frame in a report is a runtime address and the image is relocated
    // when it loads, so this line is what makes them resolvable offline.
    writer.write("Module base: ");
    writer.writeAddress(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)));
    writer.newLine();
}

bool debuggerAttached()
{
    return IsDebuggerPresent() != FALSE;
}

void installHandlers()
{
    previousFilter_ = SetUnhandledExceptionFilter(exceptionFilter);

    std::signal(SIGABRT, abortHandler);
    std::set_terminate(terminateHandler);

#ifdef HAVE_SET_ABORT_BEHAVIOR
    // Suppress the CRT's own "abort has been called" message box and its Windows
    // Error Reporting call. Both wait for a person, and the report this
    // component writes is the one worth having.
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

#ifdef HAVE_SET_INVALID_PARAMETER_HANDLER
    _set_invalid_parameter_handler(invalidParameterHandler);
#endif

#ifdef HAVE_SET_PURECALL_HANDLER
    _set_purecall_handler(purecallHandler);
#else
    static_cast<void>(&purecallHandler);
#endif
}

void uninstallHandlers()
{
    SetUnhandledExceptionFilter(previousFilter_);
    previousFilter_ = nullptr;

    std::signal(SIGABRT, SIG_DFL);
}

} // namespace CrashDump
