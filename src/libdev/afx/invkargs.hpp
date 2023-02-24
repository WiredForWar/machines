//
// I N V K A R G S . H P P
// (c) Charybdis Limited, 1996. All Rights Reserved

#ifndef _AFX_INVKARGS_INCLUDED
#define _AFX_INVKARGS_INCLUDED

#include <string>
#include <vector>

class AfxInvokeArgs : public std::vector<std::string>
{
public:
    int argc() const;

    int containsFlag(char flag) const;
    double asDouble(size_type index) const;
    double asInt(size_type index) const;
};

//////////////////////////////////////////////////////////////////////

#endif /*_AFX_INVKARGS_INCLUDED*/
