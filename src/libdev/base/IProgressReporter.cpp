#include "base/IProgressReporter.hpp"

std::size_t IProgressReporter::report(std::size_t amountOfDone, std::size_t amountOfTotal)
{
    return reportImpl(amountOfDone, amountOfTotal);
}
