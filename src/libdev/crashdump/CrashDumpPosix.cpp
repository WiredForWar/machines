#include "crashdump/internal/CrashDumpInternal.hpp"
#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/StackWalk.hpp"

#include <array>

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace CrashDump
{

namespace
{

constexpr std::array<int, 5> caughtSignals_{
    SIGABRT,
    SIGBUS,
    SIGFPE,
    SIGILL,
    SIGSEGV,
};

std::array<struct sigaction, caughtSignals_.size()> previousActions_{};

// A stack overflow raises SIGSEGV on a stack with no room left to run a
// handler, so the handler is given a stack of its own. Without this the one
// failure that most needs a backtrace is the one that produces nothing.
//
// Its size is only known at run time: SIGSTKSZ stopped being a compile-time
// constant in glibc 2.34, where it became a call to sysconf(). Allocating is
// safe where this is set up, which is from initialize(); the handler that uses
// it never allocates.
char* signalStack_{};

const char* signalName(int number)
{
    switch (number)
    {
        case SIGABRT:
            return "SIGABRT";
        case SIGBUS:
            return "SIGBUS";
        case SIGFPE:
            return "SIGFPE";
        case SIGILL:
            return "SIGILL";
        case SIGSEGV:
            return "SIGSEGV";
        default:
            return "unknown";
    }
}

void crashHandler(int number, siginfo_t* info, void* context)
{
    NativeChar path[maxReportPath]{};

    if (buildReportPath(path, maxReportPath, "crash", "txt"))
    {
        ReportWriter writer(path);

        writer.write("=== Machines crash report ===");
        writer.newLine();

        writer.write("Signal: ");
        writer.write(signalName(number));
        writer.write(" (");
        writer.writeSigned(number);
        writer.write(")");
        writer.newLine();

        if (info != nullptr)
        {
            // For SIGSEGV and SIGBUS this is the address that was accessed, not
            // the instruction that accessed it. The instruction is the first
            // frame of the trace below, taken from the interrupted context.
            writer.write("Accessed address: ");
            writer.writeAddress(reinterpret_cast<std::uintptr_t>(info->si_addr));
            writer.newLine();

            writer.write("Signal code: ");
            writer.writeSigned(info->si_code);
            writer.newLine();
        }

        writer.write("Process: ");
        writer.writeUnsigned(currentProcessId());
        writer.newLine();

        writeBuildInfo(writer);

        writer.newLine();
        writer.write("--- Call stack ---");
        writer.newLine();

        StackTrace trace;
        captureStackTraceFromContext(trace, context, nullptr);
        writeStackTrace(writer, trace);

        writeModuleInfo(writer);

        writer.newLine();
        writer.write("=== End of report ===");
        writer.newLine();

        if (writer.isOpen())
        {
            announceReport(path);
        }
    }

    // Hand the signal back to the default action, so that the exit status and
    // any core dump the system is configured to take are exactly what they
    // would have been.
    struct sigaction defaultAction{};
    defaultAction.sa_handler = SIG_DFL;
    sigemptyset(&defaultAction.sa_mask);
    sigaction(number, &defaultAction, nullptr);

    raise(number);
}

} // namespace

unsigned long long currentProcessId()
{
    return static_cast<unsigned long long>(getpid());
}

void writeModuleInfo(ReportWriter& writer)
{
    // The frames in a report are runtime addresses, and for a
    // position-independent binary they mean nothing without the address it was
    // loaded at. There is no single such address to print, so the whole map
    // goes in and scripts/symbolicate.sh picks the entry it needs out of it.
    writer.newLine();
    writer.write("--- Memory map ---");
    writer.newLine();
    writer.copyFile("/proc/self/maps");
}

bool debuggerAttached()
{
    // TracerPid in /proc/self/status is nonzero exactly while something is
    // attached, which is the only portable-enough answer Linux offers.
    const int status = open("/proc/self/status", O_RDONLY);

    if (status < 0)
    {
        return false;
    }

    char buffer[4096]{};
    const ssize_t length = read(status, buffer, sizeof(buffer) - 1);
    close(status);

    if (length <= 0)
    {
        return false;
    }

    const char* tracer = std::strstr(buffer, "TracerPid:");

    if (tracer == nullptr)
    {
        return false;
    }

    tracer += sizeof("TracerPid:") - 1;

    while (*tracer == ' ' || *tracer == '	')
    {
        ++tracer;
    }

    return *tracer != '0';
}

void installHandlers()
{
    // Twice what the platform suggests, and never less than this, so that a
    // kilobyte of path buffer and an array of frames fit with room over.
    constexpr std::size_t leastSignalStack{ 64 * 1024 };

    std::size_t signalStackSize = static_cast<std::size_t>(SIGSTKSZ) * 2;

    if (signalStackSize < leastSignalStack)
    {
        signalStackSize = leastSignalStack;
    }

    signalStack_ = static_cast<char*>(std::malloc(signalStackSize));

    if (signalStack_ != nullptr)
    {
        stack_t alternateStack{};
        alternateStack.ss_sp = signalStack_;
        alternateStack.ss_size = signalStackSize;
        alternateStack.ss_flags = 0;

        // Failing here costs the stack-overflow case and nothing else. With no
        // alternate stack installed, SA_ONSTACK is ignored and the handler runs
        // on the ordinary stack, which is where it would have run anyway.
        sigaltstack(&alternateStack, nullptr);
    }

    struct sigaction action{};
    action.sa_sigaction = crashHandler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&action.sa_mask);

    for (std::size_t i = 0; i < caughtSignals_.size(); ++i)
    {
        sigaction(caughtSignals_[i], &action, &previousActions_[i]);
    }
}

void uninstallHandlers()
{
    for (std::size_t i = 0; i < caughtSignals_.size(); ++i)
    {
        sigaction(caughtSignals_[i], &previousActions_[i], nullptr);
    }

    // Only once the handlers are back can the stack they would have run on go.
    if (signalStack_ != nullptr)
    {
        stack_t disabled{};
        disabled.ss_flags = SS_DISABLE;
        sigaltstack(&disabled, nullptr);

        std::free(signalStack_);
        signalStack_ = nullptr;
    }
}

} // namespace CrashDump
