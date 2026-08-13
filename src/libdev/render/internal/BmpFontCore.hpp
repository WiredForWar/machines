#pragma once

#include "render/Surface.hpp"
#include "system/PathName.hpp"
#include "ctl/Vector.hpp"

#include <cstddef>

struct BmpFontCoreCharData
{
    size_t offset_;
    size_t width_;
    PER_MEMBER_PERSISTENT_DEFAULT(BmpFontCoreCharData);
};

PER_READ_WRITE(BmpFontCoreCharData);
PER_DECLARE_PERSISTENT(BmpFontCoreCharData);

struct BmpFontCore
{
    BmpFontCore(const SysPathName& fontPath, std::size_t scale);

    void calculateProportionalFontWidthData(const SysPathName& persistFontPath);
    void createFromBinaryFile(const SysPathName& persistFontPath);

    bool endOfChar(int);

    RenSurface fontBmp_;
    ctl_vector<BmpFontCoreCharData> charData_;
    SysPathName fontPath_;
    size_t coreCount_{};
    size_t maxCharWidth_{};
    size_t charHeight_{};
};
