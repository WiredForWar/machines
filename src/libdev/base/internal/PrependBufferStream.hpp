/*
 * F I L T B U F F . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

/*
    DiagPrependBuffer
*/

#ifndef _UTL_FILTBUFF_HPP
#define _UTL_FILTBUFF_HPP

#include <iostream>

#include "base/base.hpp"

class UtlFilterOstream;
class DiagOstreamPrepend;

class DiagPrependBuffer : public std::streambuf
{
public:
    DiagPrependBuffer(DiagOstreamPrepend* pFilter);
    ~DiagPrependBuffer() override;

    int overflow(int c) override;
    int underflow() override;

    virtual int do_sgetn(char* buf, int len);
    virtual int do_sputn(char const* buf, int len);
    int sync() override;

private:
    // Operation deliberately revoked
    DiagPrependBuffer(const DiagPrependBuffer&);

    // Operation deliberately revoked
    DiagPrependBuffer& operator=(const DiagPrependBuffer&);

    // Operation deliberately revoked
    bool operator==(const DiagPrependBuffer&);

    DiagOstreamPrepend* pFilter_;
};

#endif

/* End FILTBUFF.HPP *************************************************/
