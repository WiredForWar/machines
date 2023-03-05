/*
 * I S T R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    PerIstream

*/

#ifndef _PER_ISTR_HPP
#define _PER_ISTR_HPP

#include <iostream>

#include "base/base.hpp"

class PerIstreamBuffer;
class PerIstreamImpl;
class PerIstreamReporter;

using std::istream;

class PerIstream
{
public:
    //  Note that istr must be capable of accepting binary data
    PerIstream( istream& istr );

    PerIstream( istream& istr, PerIstreamReporter* );
    // PRE( pReporter != NULL );

    virtual ~PerIstream();

    void read(char* pOutput, size_t length);

    //  Read a single character
    int get();

    size_t tellg() const;

    static  bool&   logRead();

private:

    friend class PerIstreamBuffer;

    PerIstreamImpl*   pImpl_;
    istream& istr_;
};


#endif

/* End ISTR.HPP *************************************************/
