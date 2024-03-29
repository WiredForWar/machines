/*
 * I S T R I M P L . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PerIstreamImpl

    A brief description of the class should go in here
*/

#ifndef _BASE_ISTRIMPL_HPP
#define _BASE_ISTRIMPL_HPP

#include "base/base.hpp"

class PerIstream;
class PerIstreamBuffer;
class IProgressReporter;

class PerIstreamImpl
// Canonical form revoked
{
public:
    PerIstreamImpl(PerIstream* pStream);
    PerIstreamImpl(PerIstream* pStream, std::istream& istr, IProgressReporter* pReporter);

    ~PerIstreamImpl();

    void logDataRead(size_t bytesRead);

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const PerIstreamImpl& t);

    PerIstreamImpl(const PerIstreamImpl&);
    PerIstreamImpl& operator=(const PerIstreamImpl&);

    friend class PerIstream;

    PerIstreamBuffer* pBuffer_;
    IProgressReporter* pReporter_;

    size_t fileSize_;
    size_t bytesRead_;

    //  The number of bytes to call back after
    size_t callBackAfter_;
};

#endif

/* End ISTRIMPL.HPP *************************************************/
