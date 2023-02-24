//
// I N V K A R G S . C P P
// (c) Charybdis Limited, 1996. All Rights Reserved

#include <algorithm>
#include "afx/invkargs.hpp"

int AfxInvokeArgs::argc() const
{
    return size();
}

int AfxInvokeArgs::containsFlag(char f) const
{
    std::string flag1 = "-x";
    flag1[1] = f;

    auto it1 = std::find(begin(), end(), flag1);
    if (it1 != end())
        return it1 - begin();

    std::string flag2 = "/x";
    flag2[1] = f;

    auto it2 = find(begin(), end(), flag2);
    if (it2 != end())
        return it2 - begin();

    return -1;
}

double AfxInvokeArgs::asDouble(size_type index) const
{
    return atof((*this)[index].c_str());
}

double AfxInvokeArgs::asInt(size_type index) const
{
    return atoi((*this)[index].c_str());
}
