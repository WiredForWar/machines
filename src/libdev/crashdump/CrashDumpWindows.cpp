#include "crashdump/internal/CrashDumpInternal.hpp"
#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/StackWalk.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <dbghelp.h>

namespace CrashDump
{

namespace
{

LPTOP_LEVEL_EXCEPTION_FILTER previousFilter_{};

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
        default:
            return "unknown";
    }
}

void writeMinidump(EXCEPTION_POINTERS* exception)
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
    information.ThreadId = GetCurrentThreadId();
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
void writeTextReport(const char* cause, const char* detail, EXCEPTION_POINTERS* exception)
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

    // Every frame below is a runtime address, and the binary is relocated at
    // load time, so this line is what makes them resolvable offline.
    writer.write("Module base: ");
    writer.writeAddress(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)));
    writer.newLine();

    writeBuildInfo(writer);

    writer.newLine();
    writer.write("--- Call stack ---");
    writer.newLine();

    StackTrace trace;

    if (exception != nullptr && exception->ContextRecord != nullptr)
    {
        captureStackTraceFromContext(trace, exception->ContextRecord, GetCurrentThread());
    }
    else
    {
        captureStackTrace(trace, 1);
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

LONG WINAPI exceptionFilter(EXCEPTION_POINTERS* exception)
{
    writeMinidump(exception);
    writeTextReport("unhandled exception", nullptr, exception);

    // Terminating here rather than continuing the search keeps the failure
    // deterministic: passing it on invites the Windows Error Reporting dialog,
    // which waits for a person and so hangs an unattended run indefinitely.
    return EXCEPTION_EXECUTE_HANDLER;
}

} // namespace

unsigned long long currentProcessId()
{
    return GetCurrentProcessId();
}

void installHandlers()
{
    previousFilter_ = SetUnhandledExceptionFilter(exceptionFilter);
}

void uninstallHandlers()
{
    SetUnhandledExceptionFilter(previousFilter_);
    previousFilter_ = nullptr;
}

} // namespace CrashDump
