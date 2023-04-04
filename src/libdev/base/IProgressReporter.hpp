/*
 * I S T R R E P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    IProgressReporter

    Reports the progress of some operation.
*/

#ifndef BASE_IPROGRESSREPORTER_HPP
#define BASE_IPROGRESSREPORTER_HPP

#include <cstddef>

class IProgressReporter
// Canonical form revoked
{
public:
    virtual ~IProgressReporter() = default;

    // Override this function to provide whatever monitoring you want.
    // The function must return the amount of work it wants to be done before
    // it is called again.
    virtual size_t report(size_t amountOfDone, size_t amountOfTotal) = 0;
};

#endif // BASE_IPROGRESSREPORTER_HPP

/* End ISTRREP.HPP **************************************************/
