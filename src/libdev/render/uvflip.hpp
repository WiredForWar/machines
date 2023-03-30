/*
 * U V F L I P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#ifndef _RENDER_UVFLIP_HPP
#define _RENDER_UVFLIP_HPP

#include "render/uvanim.hpp"

// Invert the orientation of a texture in either the u or v directions or in
// both.  This will almost certainly not produce a sensible visual result if
// the polygon in question is not covered by a whole number of repeats.
// This class is dedicated to Josh & Irenee's pet rabbit.
class RenUVFlip : public RenUVTransform
{
public:
    RenUVFlip();
    ~RenUVFlip() override;

    enum Flip
    {
        NONE,
        U,
        V,
        BOTH
    };
    void flipped(Flip);
    Flip flipped() const;

    // These toggle the flip state, so that repeated calls to the same function
    // will alternate the texture's orientation.
    void invertU();
    void invertV();
    void invertBoth();

    // PRE(uOut && vOut);
    void transform(float uIn, float vIn, float* uOut, float* vOut) const override;
    void print(std::ostream&) const override;

    // A virtual copy operation.  _NEW is used to allocate the return value.
    // The caller is responsible for calling _DELETE.
    RenUVTransform* clone() const override;

private:
    bool uFlipped_, vFlipped_;
};

#endif

/* End UVFLIP.HPP ***************************************************/
