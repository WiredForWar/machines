#include <iostream>
#include "render/surface.hpp"

#pragma GCC diagnostic ignored "-Wunused-parameter"

RenSurface RenSurface::createSharedSurface(const std::string& bitmapName, const RenSurface& surf)
{
    RenSurface defaultSurface;
    return defaultSurface;
}

void RenSurface::mgrIncrementRefCount(Ren::TexId texId)
{
    return;
}

void RenSurface::mgrDecrementRefCount(Ren::TexId texId)
{
    return;
}

#pragma GCC diagnostic warning "-Wunused-parameter"
