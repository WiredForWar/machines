/*
 * I S T R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    PerIstream

*/

#ifndef _PER_ISTR_HPP
#define _PER_ISTR_HPP

#include "base/base.hpp"

#include <istream>

class PerIstreamBuffer;
class PerIstreamImpl;
class IProgressReporter;

class PerIstream
{
public:
    //  Note that istr must be capable of accepting binary data
    PerIstream(std::istream& istr);

    PerIstream(std::istream& istr, IProgressReporter*);
    // PRE( pReporter != NULL );

    virtual ~PerIstream();

    void read(void* pOutput, size_t length);

    //  Read a single character
    int get();

    size_t tellg() const;

    static bool& logRead();

private:
    friend class PerIstreamBuffer;

    PerIstreamImpl* pImpl_;
    std::istream& istr_;
};

#endif

/* End ISTR.HPP *************************************************/
