#include "crashdump/CrashTest.hpp"

#include <stdexcept>
#include <string_view>

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace CrashDump
{

namespace
{

// The optimiser would otherwise fold these into their caller, and the frame the
// symbolication check looks for would not exist in the release binary it checks.
// There is no standard spelling for this, and the whole point of the functions
// is to be findable by name in a call stack.
#ifdef _MSC_VER
#define CRASHDUMP_NOINLINE __declspec(noinline)
#else
#define CRASHDUMP_NOINLINE __attribute__((noinline))
#endif

// The pointer is itself volatile, not merely a pointer to volatile. Given a
// value it can prove is null, a compiler is entitled to treat the store as
// unreachable and delete it outright -- and GCC does, which left the trigger
// returning normally and provoking nothing at all. Making the pointer volatile
// forces it to be loaded at run time and takes that proof away.
//
// MSVC kept the store, so this only ever showed up on the toolchain that ships.
volatile int* volatile nullPointer_{ nullptr };

CRASHDUMP_NOINLINE void crashTestWriteToNullPointer()
{
    *nullPointer_ = 1;
}

CRASHDUMP_NOINLINE void crashTestAbort()
{
    std::abort();
}

CRASHDUMP_NOINLINE void crashTestThrow()
{
    throw std::runtime_error("crash test");
}

// Exhausting the stack takes more care than it looks. The frame is volatile so
// that it cannot be optimised away, and the recursive call's result is consumed
// afterwards so that it cannot be a tail call: given a tail call, both compilers
// reuse the single frame and emit a jump back to the top, and the result is a
// function that spins at full speed forever having overflowed nothing.
//
// Both compilers also notice that this can only end in a stack overflow, which
// is the entire purpose of it.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4717)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
#endif

CRASHDUMP_NOINLINE unsigned long long crashTestRecurse(unsigned long long depth)
{
    volatile char frame[4096]{};
    frame[0] = static_cast<char>(depth);

    return crashTestRecurse(depth + frame[0] + 1) + frame[0];
}

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef _WIN32
CRASHDUMP_NOINLINE void crashTestInvalidParameter()
{
    // The C runtime rejects this by fast-failing rather than by returning an
    // error, which is the path _set_invalid_parameter_handler exists to catch.
    const char* format = nullptr;
    std::printf(format);
}
#endif

std::string_view crashTestArgument(int argc, char* argv[])
{
    constexpr std::string_view option{ "--crash-test=" };

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument{ argv[i] };

        if (argument.starts_with(option))
        {
            return argument.substr(option.size());
        }
    }

    return {};
}

} // namespace

bool runCrashTestIfRequested(int argc, char* argv[])
{
    const std::string_view kind = crashTestArgument(argc, argv);

    if (kind.empty())
    {
        return false;
    }

    bool recognised{ true };

    if (kind == "segv")
    {
        crashTestWriteToNullPointer();
    }
    else if (kind == "abort")
    {
        crashTestAbort();
    }
    else if (kind == "terminate")
    {
        crashTestThrow();
    }
    else if (kind == "stack-overflow")
    {
        crashTestRecurse(1);
    }
#ifdef _WIN32
    else if (kind == "invalid-parameter")
    {
        crashTestInvalidParameter();
    }
#endif
    else
    {
        recognised = false;
    }

    // Getting here means the check did not do what it claims, so it may neither
    // look like success nor start the game. The two ways of getting here are
    // told apart because they call for opposite responses: an unknown kind is a
    // typo in the caller, while a known one that returned is a trigger the
    // optimiser has quietly removed -- which provokes nothing at all and would
    // otherwise pass for a working crash handler.
    std::_Exit(recognised ? 4 : 2);
}

} // namespace CrashDump
