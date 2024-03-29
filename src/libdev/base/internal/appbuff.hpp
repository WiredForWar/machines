/*
 * A P P B U F F . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    BaseAppendBuffer

    Internal support class for BaseAppendOstream.
*/

#ifndef _BASE_APPBUFF_HPP
#define _BASE_APPBUFF_HPP

#include <iostream>

#include "base/base.hpp"

class BaseAppendBuffer : public std::streambuf
{
public:
    BaseAppendBuffer(const char* fileName);
    // PRE( fileName != NULL );

    BaseAppendBuffer();

    ~BaseAppendBuffer() override;

    void name(const char* fileName);

    void clear();

    int overflow(int c) override;
    int underflow() override;

    virtual int do_sgetn(char* buf, int len);
    virtual int do_sputn(char const* buf, int len);
    int sync() override;

private:
    // Operation deliberately revoked
    BaseAppendBuffer(const BaseAppendBuffer&);

    // Operation deliberately revoked
    BaseAppendBuffer& operator=(const BaseAppendBuffer&);

    // Operation deliberately revoked
    bool operator==(const BaseAppendBuffer&);

    char* pFileName_;
};

#endif

/* End APPBUFF.HPP *************************************************/
