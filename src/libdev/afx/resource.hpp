/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#pragma once

#include "system/pathname.hpp"

#include <map>

// Represents a resource file which can contain strings and/or bitmaps.
class AfxResourceLib
{
public:
    AfxResourceLib() = default;
    virtual ~AfxResourceLib() = default;

    bool addStringsFromFile(const SysPathName& path);

    // The "get" is not ideal, but "string" conflicts with the type name.
    std::string getString(const uint id) const;

private:
    std::map<uint, std::string> resourceStrings_;
    SysPathName fileName_;
};
