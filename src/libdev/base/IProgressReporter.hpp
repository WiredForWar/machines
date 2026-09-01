#pragma once

#include <cstddef>

// Reports the progress of a long operation, and paces how often it is asked to.
//
// The public entry point is not virtual: it does the bookkeeping every reporter
// owes regardless of what it displays, then defers to the implementation.
class IProgressReporter
{
public:
    virtual ~IProgressReporter() = default;

    // Say that amountOfDone of amountOfTotal is finished. Answers how much more
    // work should be done before reporting again, so that a cheap operation is
    // not swamped by the reporting of it.
    std::size_t report(std::size_t amountOfDone, std::size_t amountOfTotal);

protected:
    // Monitor the operation however the implementation needs to, and answer the
    // amount of work wanted before the next call.
    virtual std::size_t reportImpl(std::size_t amountOfDone, std::size_t amountOfTotal) = 0;
};
