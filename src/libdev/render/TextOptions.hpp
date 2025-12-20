#ifndef RENDER_TEXT_OPTIONS_HPP
#define RENDER_TEXT_OPTIONS_HPP

#include "Alignment.hpp"
#include "colour.hpp"

namespace Render
{

// Text render options
class TextOptions
{
public:
    TextOptions() = default;
    TextOptions(const RenColour& color, Alignment alignment = AlignLeft)
        : alignment_(alignment)
        , color_(color)
    {
    }

    Alignment alignment() const { return alignment_; }
    void setAlignment(Alignment alignment) { alignment_ = alignment; }

    RenColour color() const { return color_; }
    void setColor(RenColour color) { color_ = color; }

    int letterSpacing() const { return letterSpacing_; }
    void setLetterSpacing(int value) { letterSpacing_ = value; };

    int shadowX() const { return shadowX_; }
    int shadowY() const { return shadowY_; }
    bool hasShadow() const { return shadowX_ || shadowY_; }
    RenColour shadowColor() const { return secondColor_; }

    void setShadow(int x, int y, RenColour color)
    {
        shadowX_ = x;
        shadowY_ = y;
        secondColor_ = color;
    }

    bool hasOutline() const { return outlineThickness_ > 0; }
    int outlineThickness() const { return outlineThickness_; }
    RenColour outlineColor() const { return outlineColor_; }

    void setOutline(int thickness, RenColour color)
    {
        outlineThickness_ = thickness;
        outlineColor_ = color;
    }

    bool underline() const { return underline_; }
    void setUnderline(bool underline)
    {
        underline_ = underline;
    }

private:
    Alignment alignment_{};
    int letterSpacing_{};
    int shadowX_{};
    int shadowY_{};
    int outlineThickness_{};
    bool underline_{};

    RenColour color_;
    RenColour secondColor_;
    RenColour outlineColor_;
};

} // Render namespace

#endif // RENDER_TEXT_OPTIONS_HPP
