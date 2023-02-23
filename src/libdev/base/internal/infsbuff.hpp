/*
 * I S T R B U F F . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

/*
    BaseInfiniteStreamBuffer

    This is a support class for BaseLogBuffer. The class IS-A
    streambuf which puts its output into a buffer that will
    increase its size as necessary.
*/

#ifndef _ISTRBUFF_HPP
#define _ISTRBUFF_HPP

#include "base/base.hpp"

#include "base/internal/lstrbuff.hpp"

class BaseInfiniteStreamBuffer : public BaseLogStreamBuffer
{
public:
    BaseInfiniteStreamBuffer();
    ~BaseInfiniteStreamBuffer() override;

    void clear() override;

    int overflow(int c) override;
    int underflow() override;

    virtual int do_sgetn(char* buf, int len);
    virtual int do_sputn(char const* buf, int len);
    int sync() override;

private:
    // Operation deliberately revoked
    BaseInfiniteStreamBuffer(const BaseInfiniteStreamBuffer&);

    // Operation deliberately revoked
    BaseInfiniteStreamBuffer& operator=(const BaseInfiniteStreamBuffer&);

    // Operation deliberately revoked
    bool operator==(const BaseInfiniteStreamBuffer&);

    size_t nCharactersInBuffer() const override;
    static size_t bufferIncrement();

    char* pBuffer_;
    size_t bufferSize_;
    size_t nCharactersOutput_;
};

#endif

/* End ISTRBUFF.HPP *************************************************/
