/*
 * T E X B O D Y . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_TEXBODY_HPP
#define _RENDER_TEXBODY_HPP

#include "base/base.hpp"
#include "system/pathname.hpp"
#include "render/surfmgr.hpp"
#include "render/internal/surfbody.hpp"

class RenColour;
class RenDevice;

// A surface with a few extra texture-specific members, most importantly, the
// Direct3D texture handle.
class RenITexBody : public RenISurfBody
{
public:
    ~RenITexBody() override;

    // This overrides the RenISurfBody::read.  It chooses the best matching
    // pixel format for a texture.  It also handles alpha maps and recognises
    // naming conventions for colour-keying, alpha and bilinear blending.
    bool read(const std::string& name) override;
    RenITexBody* castToTexBody() override;
    const RenITexBody* castToTexBody() const override;
    void print(std::ostream&) const override;

    bool hasAlphaTransparency() const;
    bool usesBilinear() const;

    uint hitsPerFrame() const;
    void incHits();
    void resetHits();

    void CLASS_INVARIANT;

    // TBD: sort out access to ctor(s) via friends or sth.
    RenITexBody();

private:
    bool loaded_;
    uint hitsPerFrame_;
    bool bilinear_, alpha_;

    // Operations deliberately revoked (these objects are ref counted).
    RenITexBody(const RenITexBody&);
    RenITexBody& operator=(const RenITexBody&);
    bool operator==(const RenITexBody&);
};

#ifdef _INLINE
#include "internal/texbody.ipp"
#endif

#endif

/* End TEXBODY.HPP **************************************************/
