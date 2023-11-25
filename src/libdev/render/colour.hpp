/*
 * C O L O U R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_COLOUR_HPP
#define _RENDER_COLOUR_HPP

#include "base/base.hpp"
#include "base/persist.hpp"

class RenColour
{
public:
    constexpr RenColour(float grey = 0);
    constexpr RenColour(float r, float g, float b, float a = 1); // PRE(0 <= a <= 1);

    float r() const;
    float g() const;
    float b() const;
    float a() const; // alpha transparency
    void r(float v); // PRE(0 <= v);
    void g(float v); // PRE(0 <= v);
    void b(float v); // PRE(0 <= v);
    void a(float v); // PRE(0 <= v <= 1);

    // Get the alpha packed into the format used by Direct3D, i.e. as the
    // most-significant-byte of a four-byte word.
    uint32_t packedAlpha() const;

    // Returns true if alpha is less than 1.0.
    bool isTransparent() const;

    bool operator==(const RenColour& c) const;
    bool operator!=(const RenColour& c) const;
    RenColour& operator+=(const RenColour&);
    RenColour& operator-=(const RenColour&);
    RenColour& operator*=(float s);
    RenColour& operator/=(float s); // PRE(s!=0);
    RenColour& operator*=(const RenColour&);

    // Do a linear interpolation between c1 and c2.  This colour will be
    // set to c1 + (c2-c1) * (i-i1)/(i2-i1).  Returns *this.
    RenColour& linearInterpolate(float i, const RenColour& c1, float i1, const RenColour& c2, float i2);

    static const RenColour& black();
    static const RenColour& white();
    static const RenColour& red();
    static const RenColour& green();
    static const RenColour& blue();
    static const RenColour& yellow();
    static const RenColour& magenta();
    static const RenColour& cyan();

    void CLASS_INVARIANT;
    friend std::ostream& operator<<(std::ostream& o, const RenColour& t);
    friend std::istream& operator>>(std::istream& i, RenColour& t);

    PER_MEMBER_PERSISTENT_DEFAULT(RenColour)
    PER_FRIEND_READ_WRITE(RenColour)

private:
    bool lessThanOrEq(float d1, float d2) const;

    struct IColour
    {
        constexpr IColour(float r, float g, float b, float a);

        constexpr void packAlpha(float a);
        float unpackAlpha() const;

        float r_ = 0;
        float g_ = 0;
        float b_ = 0;
        uint32 a_ = 0;
    };

    IColour rep_;
};

PER_DECLARE_PERSISTENT(RenColour);

// A class which takes a an input colour and produces an output colour as
// some function of the input one.
class RenColourTransform
{
public:
    // PRE(out);
    virtual void transform(const RenColour& in, RenColour* out) const = 0;
    virtual ~RenColourTransform();
};

inline constexpr RenColour::RenColour(float grey)
    : rep_(grey, grey, grey, 1)
{
}

inline constexpr RenColour::RenColour(float r, float g, float b, float a)
    : rep_(r, g, b, a)
{
}

inline constexpr RenColour::IColour::IColour(float r, float g, float b, float a)
    : r_(r)
    , g_(g)
    , b_(b)
{
    packAlpha(a);
}

inline constexpr void RenColour::IColour::packAlpha(float a)
{
    if (a >= 1.0)
        a_ = 0xff000000L;
    else if (a <= 0.0)
        a_ = 0;
    else
        a_ = static_cast<uint32_t>(255 * a) << 24;
}

#ifdef _INLINE
#include "render/colour.ipp"
#endif

#endif

/* End COLOUR.HPP ***************************************************/
