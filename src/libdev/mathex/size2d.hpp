#ifndef MATHEX_SIZE2D_HPP
#define MATHEX_SIZE2D_HPP

#include "mathex/mathex.hpp"

class MexSize2d
{
public:
    constexpr MexSize2d(MATHEX_SCALAR width, MATHEX_SCALAR height);
    MexSize2d() = default;

    ~MexSize2d() = default;

    MexSize2d& operator=(const MexSize2d& rhs);
    constexpr bool operator==(const MexSize2d&) const;
    constexpr bool operator!=(const MexSize2d&) const;

    constexpr MexSize2d& operator*=(MATHEX_SCALAR multiplier);

    constexpr MATHEX_SCALAR width() const;
    constexpr MATHEX_SCALAR height() const;
    void setWidth(MATHEX_SCALAR widthNew);
    void setHeight(MATHEX_SCALAR heightNew);

    friend std::ostream& operator<<(std::ostream& o, const MexSize2d& t);

private:
    //The coordinates
    MATHEX_SCALAR width_ = 0;
    MATHEX_SCALAR height_ = 0;
};

inline MexSize2d& MexSize2d::operator=(const MexSize2d& rhs)
{
    width_ = rhs.width_;
    height_ = rhs.height_;

    POST(*this == rhs);

    return *this;
}

constexpr inline bool MexSize2d::operator==(const MexSize2d& rhs) const
{
    return (width_ == rhs.width_) && (height_ == rhs.height_);
}

constexpr inline MexSize2d& MexSize2d::operator*=(MATHEX_SCALAR multiplier)
{
    width_ *= multiplier;
    height_ *= multiplier;
    return *this;
}

constexpr inline MexSize2d operator*(MexSize2d rhs, MATHEX_SCALAR c)
{
    return rhs *= c;
}

constexpr inline MexSize2d::MexSize2d(MATHEX_SCALAR w, MATHEX_SCALAR h)
    : width_(w)
    , height_(h)
{
}

constexpr inline MATHEX_SCALAR MexSize2d::width() const
{
    return width_;
}

constexpr inline MATHEX_SCALAR MexSize2d::height() const
{
    return height_;
}

inline void MexSize2d::setWidth(MATHEX_SCALAR widthNew)
{
    width_ = widthNew;
}

inline void MexSize2d::setHeight(MATHEX_SCALAR heightNew)
{
    height_ = heightNew;
}

#endif // MATHEX_SIZE2D_HPP
