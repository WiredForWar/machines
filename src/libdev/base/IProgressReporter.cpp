#include "base/IProgressReporter.hpp"

#include "crashdump/CrashDump.hpp"

std::size_t IProgressReporter::report(std::size_t amountOfDone, std::size_t amountOfTotal)
{
    // A load is the one thing that legitimately keeps the main loop from
    // turning for a long time, and this is the only point every load passes
    // through while it is working. Saying so is what separates a slow load from
    // a hang, and it belongs here rather than in whichever implementation
    // happens to draw a bar: the console's load_planet uses one that draws
    // nothing at all.
    CrashDump::heartbeat();

    return reportImpl(amountOfDone, amountOfTotal);
}
