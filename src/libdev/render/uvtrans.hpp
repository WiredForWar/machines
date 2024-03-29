/*
 * U V T R A N S . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_UVTRANS_HPP
#define _RENDER_UVTRANS_HPP

#include "render/uvanim.hpp"
#include "mathex/vec2.hpp"

class RenUVTranslation : public RenUVTransform
{
public:
    RenUVTranslation();
    explicit RenUVTranslation(const MexVec2&);
    ~RenUVTranslation() override;

    // PRE(uOut && vOut);
    void transform(float uIn, float vIn, float* uOut, float* vOut) const override;
    void print(std::ostream&) const override;

    // A virtual copy operation.  _NEW is used to allocate the return value.
    // The caller is responsible for calling _DELETE.
    RenUVTransform* clone() const override;

    void translation(const MexVec2&);
    const MexVec2& translation() const;

    PER_MEMBER_PERSISTENT_DEFAULT_VIRTUAL(RenUVTranslation);
    PER_FRIEND_READ_WRITE(RenUVTranslation);

private:
    MexVec2 translation_;
};

PER_DECLARE_PERSISTENT(RenUVTranslation);

#endif

/* End UVTRANS.HPP **************************************************/
