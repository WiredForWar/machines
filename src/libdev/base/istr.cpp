/*
 * I S T R . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <sstream>
#include <iostream>

#include "base/istr.hpp"
#include "base/persist.hpp"
#include "base/internal/istrbuff.hpp"
#include "base/internal/istrimpl.hpp"

PerIstream::PerIstream(istream& istr)
    : pImpl_(_NEW(PerIstreamImpl(this)))
    , istr_(istr)
// istream( istr )
// istream( pImpl_->pBuffer_ )
{
    Persistence::instance().registerOpenIstream();
}

PerIstream::PerIstream(istream& istr, PerIstreamReporter* pReporter)
    : pImpl_(_NEW(PerIstreamImpl(this, istr, pReporter)))
    , istr_(istr)
{
    PRE(pReporter != nullptr);

    Persistence::instance().registerOpenIstream();
}

PerIstream::~PerIstream()
{
    _DELETE(pImpl_);

    Persistence::instance().registerCloseIstream();
}

void PerIstream::read(char* pOutput, size_t length)
{
    istr_.read(pOutput, length);
    pImpl_->logDataRead(length);
}

//  Read a single character
int PerIstream::get()
{
    pImpl_->logDataRead(1);

    int ch = istr_.get();

    if (logRead())
        PER_READ_INDENT_STREAM("$" << std::hex << (int)ch << "$" << std::dec);

    return ch;
}

size_t PerIstream::tellg() const
{
    return istr_.tellg();
}

// static
bool& PerIstream::logRead()
{
    static bool logRead_ = false;

    return logRead_;
}

//
// void    PerOstream::filterCharacters( const char* pBuf, size_t nChars )
// {
//     outputCharacters( pBuf, nChars );
// }
//
// void    PerOstream::outputCharacters( const char* pBuf, size_t nChars )
// {
//     ostr_.write( pBuf, nChars );
// }

/* End ISTR.CPP *************************************************/
