/*
 * F I X D C U R S . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_FIXDCURS_HPP
#define _RENDER_FIXDCURS_HPP

#include "base/base.hpp"
#include "render/cursor2d.hpp"
#include "render/surface.hpp"

// A 2D cursor which has one unanimated bitmap image.
class RenFixedCursor2d : public RenCursor2d
{
public:
    RenFixedCursor2d(const RenSurface&);
    ~RenFixedCursor2d() override;

    const RenSurface& currentBitmap() const override;
    void print(ostream& o) const override;

private:
    const RenSurface surf_;
};

#endif

/* End FIXDCURS.HPP *************************************************/
