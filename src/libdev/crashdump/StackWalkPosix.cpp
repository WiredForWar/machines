#include "crashdump/internal/StackWalk.hpp"

#include <cerrno>
#include <csignal>

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <ucontext.h>
#include <unwind.h>

namespace CrashDump
{

namespace
{

// SIGUSR2 rather than SIGUSR1: the game does not use either today, but SIGUSR1
// is the one a profiler or a debugger is most likely to claim, and a collision
// would send this handler somebody else's interrupts.
constexpr int captureSignal{ SIGUSR2 };

// A thread that has not answered in this long is not going to.
constexpr int captureTimeoutSeconds{ 2 };

pthread_t watchedThread_{};
bool watchedThreadSet_{};

StackTrace capturedTrace_{};
sem_t captureDone_{};

// The address the interrupted thread was executing at. The unwind below starts
// from inside the handler, so its innermost entries describe the handler and
// the signal trampoline rather than the fault, and the one frame that matters
// most has to come from the context the kernel handed over.
std::uintptr_t faultingProgramCounter(const ucontext_t& context)
{
// These name the kernel's signal context rather than anything one C library
// invented, so glibc, musl and Bionic all spell them the same way. Selecting on
// the architecture alone is deliberate: a library that turns out not to have
// them should fail to build and say so, because the alternative is a guard that
// quietly drops the faulting instruction and leaves a report that looks whole.
#if defined(__x86_64__)
    return static_cast<std::uintptr_t>(context.uc_mcontext.gregs[REG_RIP]);
#elif defined(__i386__)
    return static_cast<std::uintptr_t>(context.uc_mcontext.gregs[REG_EIP]);
#elif defined(__aarch64__)
    return static_cast<std::uintptr_t>(context.uc_mcontext.pc);
#elif defined(__arm__)
    return static_cast<std::uintptr_t>(context.uc_mcontext.arm_pc);
#else
    static_cast<void>(context);
    return 0;
#endif
}

struct UnwindState
{
    StackTrace* trace{};
    std::size_t framesToSkip{};
};

_Unwind_Reason_Code collectFrame(_Unwind_Context* context, void* argument)
{
    UnwindState& state = *static_cast<UnwindState*>(argument);

    const std::uintptr_t address = static_cast<std::uintptr_t>(_Unwind_GetIP(context));

    if (address == 0)
    {
        return _URC_END_OF_STACK;
    }

    if (state.framesToSkip > 0)
    {
        --state.framesToSkip;
        return _URC_NO_REASON;
    }

    if (state.trace->frameCount >= StackTrace::maxFrames)
    {
        return _URC_END_OF_STACK;
    }

    state.trace->frames[state.trace->frameCount] = address;
    ++state.trace->frameCount;

    return _URC_NO_REASON;
}

// The unwinder every C++ toolchain already ships, in preference to backtrace().
// <execinfo.h> is a glibc facility that Bionic does not have at all, so on
// Android backtrace() is not a slower way of getting the frames -- it is no way
// of getting them, and a report would come out whole but empty. This is what
// backtrace() calls underneath on glibc anyway, and it reads the unwind tables
// that -funwind-tables exists to emit.
//
// It is not on the list of async-signal-safe functions, because the first call
// may resolve the unwinder through the dynamic loader. warmUpStackWalk() makes
// that call from ordinary code, so that by the time a handler runs there is
// nothing left to resolve.
void capture(StackTrace& trace, std::size_t skipFrames)
{
    UnwindState state{ &trace, skipFrames };

    _Unwind_Backtrace(collectFrame, &state);
}

// Runs on the watched thread. sem_post is on the short list of functions that
// stay safe in a signal handler, which is why the answer is handed back through
// a semaphore rather than a condition variable.
void captureHandler(int number, siginfo_t* info, void* context)
{
    static_cast<void>(number);
    static_cast<void>(info);
    static_cast<void>(context);

    capture(capturedTrace_, 2);

    sem_post(&captureDone_);
}

} // namespace

void captureStackTrace(StackTrace& trace, std::size_t skipFrames)
{
    // One extra for this function's own frame.
    capture(trace, skipFrames + 1);
}

void captureStackTraceFromContext(StackTrace& trace, void* context, void* thread)
{
    // POSIX delivers the signal on the thread that raised it, so the walk is
    // always of the caller's own stack.
    static_cast<void>(thread);

    if (context != nullptr)
    {
        const std::uintptr_t programCounter = faultingProgramCounter(*static_cast<const ucontext_t*>(context));

        if (programCounter != 0)
        {
            trace.frames[trace.frameCount] = programCounter;
            ++trace.frameCount;
        }
    }

    // Two frames of handler machinery sit above the trampoline; dropping them
    // puts the faulting function immediately below the address taken above.
    capture(trace, 3);
}

// Resolve whatever the unwinder needs from the dynamic loader now, while
// allocation and locking are still safe.
void warmUpStackWalk()
{
    StackTrace trace;
    captureStackTrace(trace, 0);
}

void rememberWatchedThread()
{
    sem_init(&captureDone_, 0, 0);

    watchedThread_ = pthread_self();
    watchedThreadSet_ = true;
}

bool captureWatchedThreadStackTrace(StackTrace& trace)
{
    if (! watchedThreadSet_)
    {
        return false;
    }

    // There is no portable way to read another thread's stack, so the watched
    // thread is asked to read its own: a signal interrupts it wherever it is,
    // the handler captures the stack it was interrupted on, and this thread
    // waits for the answer. A thread wedged in the kernel with the signal
    // blocked will never answer, which is what the timeout is for -- and a hang
    // is exactly when that is worth allowing for.
    struct sigaction action{};
    action.sa_sigaction = captureHandler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&action.sa_mask);

    struct sigaction previous{};

    if (sigaction(captureSignal, &action, &previous) != 0)
    {
        return false;
    }

    capturedTrace_ = StackTrace{};

    bool captured{};

    if (pthread_kill(watchedThread_, captureSignal) == 0)
    {
        timespec deadline{};

        if (clock_gettime(CLOCK_REALTIME, &deadline) == 0)
        {
            deadline.tv_sec += captureTimeoutSeconds;

            while (sem_timedwait(&captureDone_, &deadline) != 0)
            {
                if (errno != EINTR)
                {
                    break;
                }
            }

            captured = capturedTrace_.frameCount != 0;
        }
    }

    sigaction(captureSignal, &previous, nullptr);

    if (captured)
    {
        trace = capturedTrace_;
    }

    return captured;
}

} // namespace CrashDump
