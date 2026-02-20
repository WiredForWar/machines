/*
 * P A I N T E R . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/Painter.hpp"

#include "render/colour.hpp"
#include "render/surface.hpp"
#include "render/Font.hpp"
#include "render/TextOptions.hpp"
#include "render/device.hpp"
#include "render/internal/surfbody.hpp"
#include "render/internal/colpack.hpp"
#include "render/internal/vtxdata.hpp"
#include "render/internal/FontImpl.hpp"
#include "render/internal/RenScopedImmediateCommands.hpp"

#include <cmath>
#include <vector>

namespace Ren {

Painter::Painter(RenSurface& target)
    : target_(target)
{
}

void Painter::filledRectangle(const Rect& area, const RenColour& colour) const
{
    PRE(!target_.readOnly());
    PRE(RenDevice::current());
    PRE(RenDevice::current()->display());

    if (target_.isNull())
        return;

    RenScopedImmediateCommands guard(RenDevice::current());
    Rect srcArea(Size(1, 1));
    RenISurfBody emptySurf;
    RenDevice* dev = RenDevice::current();

    uint packedColour = packColour(colour.r(), colour.g(), colour.b(), colour.a());
    const bool backgroundColour = packedColour == 0xFFFF00FF;
    const BlitMode blitMode = backgroundColour ? BlitMode::ZeroZero : BlitMode::AlphaBlend;

    if (target_.isOffscreen())
    {
        dev->renderToTextureMode(target_.handle(), target_.width(), target_.height());
        dev->renderSurface(&emptySurf, srcArea, area, target_.width(), target_.height(), packedColour, blitMode);
        dev->renderToTextureMode(NullTexId, 0, 0);
    }
    else
    {
        dev->renderSurface(&emptySurf, srcArea, area, 0, 0, packedColour, blitMode);
    }
}

void Painter::hollowRectangle(const Rect& area, const RenColour& colour, int thickness) const
{
    PRE(!target_.readOnly());
    PRE(thickness > 0);

    Rect orderedArea = area;

    if (orderedArea.width < 0)
    {
        orderedArea.originX += orderedArea.width;
        orderedArea.width = -orderedArea.width;
    }

    if (orderedArea.height < 0)
    {
        orderedArea.originY += orderedArea.height;
        orderedArea.height = -orderedArea.height;
    }

    const int x = orderedArea.originX;
    const int y = orderedArea.originY;
    const int w = orderedArea.width;
    const int h = orderedArea.height;
    const int t = thickness;

    // Top
    filledRectangle(Rect(x, y, w, t), colour);
    // Bottom
    filledRectangle(Rect(x, y + h - t, w, t), colour);
    // Left
    filledRectangle(Rect(x, y + t, t, h - 2 * t), colour);
    // Right
    filledRectangle(Rect(x + w - t, y + t, t, h - 2 * t), colour);
}

void Painter::ellipse(const Rect& area, const RenColour& outline, const RenColour& fill) const
{
    PRE(!target_.readOnly());
    RenScopedImmediateCommands guard(RenDevice::current());

    int cx, cy, rx, ry;
    uint packedColour = packColour(fill.r(), fill.g(), fill.b(), fill.a());
    rx = area.width / 2;
    ry = area.height / 2;
    cx = area.originX + rx;
    cy = area.originY + ry;
    std::vector<RenIVertex> vertices;
    vertices.reserve(10);

    float i, inc, endAngle;
    endAngle = 3.1415f * 2;
    inc = endAngle / 10;
    i = 0;
    while (i <= endAngle)
    {
        RenIVertex vtx;
        vtx.x = rx * std::cos(i) + cx;
        vtx.y = ry * std::sin(i) + cy;
        vtx.color = packedColour;
        vertices.push_back(vtx);
        i += inc;
    }

    RenDevice::current()->recordCommand(Command::setCullFace(false));
    RenDevice* dev = RenDevice::current();
    if (target_.isOffscreen())
    {
        dev->renderToTextureMode(target_.handle(), target_.width(), target_.height());
        dev->renderScreenspace(vertices.data(), vertices.size(), PrimitiveTopology::TriangleFan, target_.width(), -target_.height());
        dev->renderToTextureMode(NullTexId, 0, 0);
    }
    else
        dev->renderScreenspace(vertices.data(), vertices.size(), PrimitiveTopology::TriangleFan, target_.width(), target_.height());
}

void Painter::line(const Point& from, const Point& to, const RenColour& colour, int thickness) const
{
    PRE(!target_.readOnly());
    PRE(thickness > 0);
    RenScopedImmediateCommands guard(RenDevice::current());

    RenIVertex vtx[2]{};
    uint packedColour = packColour(colour.r(), colour.g(), colour.b(), colour.a());

    vtx[0].x = from.x;
    vtx[0].y = from.y;
    vtx[0].z = 0.0;
    vtx[0].w = 0.1;
    vtx[0].tu = vtx[0].tv = 0.1;
    vtx[0].color = packedColour;
    vtx[0].specular = 0;

    vtx[1].x = to.x;
    vtx[1].y = to.y;
    vtx[1].z = 0.0;
    vtx[1].w = 0.1;
    vtx[1].tu = vtx[1].tv = 0.1;
    vtx[1].color = packedColour;
    vtx[1].specular = 0;

    RenDevice* dev = RenDevice::current();
    dev->recordCommand(Command::setLineWidth(static_cast<float>(thickness)));
    if (target_.isOffscreen())
    {
        dev->renderToTextureMode(target_.handle(), target_.width(), target_.height());
        dev->renderScreenspace(vtx, 2, PrimitiveTopology::LineStrip, target_.width(), -target_.height());
        dev->renderToTextureMode(NullTexId, 0, 0);
    }
    else
        dev->renderScreenspace(vtx, 2, PrimitiveTopology::LineStrip, target_.width(), target_.height());
    dev->recordCommand(Command::setLineWidth(1.0f));
}

void Painter::horizontalLine(const Point& start, int length, const RenColour& colour, int thickness) const
{
    if (thickness > 1)
    {
        const Point adjusted { start.x, start.y - thickness / 2 };
        line(adjusted, Point { adjusted.x + length, adjusted.y }, colour, thickness);
    }
    else
    {
        line(start, Point { start.x + length, start.y }, colour, thickness);
    }
}

void Painter::verticalLine(const Point& start, int height, const RenColour& colour, int thickness) const
{
    line(start, Point { start.x, start.y + height }, colour, thickness);
}

void Painter::drawText(int x, int y, std::string_view text, const Font& font, const TextOptions& options) const
{
    RenScopedImmediateCommands guard(RenDevice::current());

    struct UnderlineSegment
    {
        int x1{};
        int x2{};
        int y{};
    };

    std::vector<UnderlineSegment> underlineSegments;

    std::vector<RenIVertex> vertices;
    {
        int passes = 1;
        if (options.hasShadow())
        {
            passes += 1;
        }
        if (options.hasOutline())
        {
            const int thickness = options.outlineThickness();
            int outlinePasses = 0;
            for (int ring = 1; ring <= thickness; ++ring)
            {
                outlinePasses += ring * 8;
            }
            passes += outlinePasses;
        }

        int expectedVerticesNumber = text.size() * 6 * passes;
        vertices.reserve(expectedVerticesNumber);
    }

    RenColour col = options.color();
    uint fontColor = packColour(col.r(), col.g(), col.b(), 1.0);

    uint secondColor = 0;

    if (options.hasShadow())
    {
        RenColour unpacked = options.shadowColor();
        secondColor = packColour(unpacked.r(), unpacked.g(), unpacked.b(), 1.0);
    }

    uint outlineColor = 0;
    if (options.hasOutline())
    {
        RenColour unpacked = options.outlineColor();
        outlineColor = packColour(unpacked.r(), unpacked.g(), unpacked.b(), 1.0);
    }

    const FontImpl& fontImpl = *FontImpl::get(&font);
    const FontImpl::CharData* charData = nullptr;

    y += fontImpl.ascender();

    if (options.alignment() & AlignRight)
    {
        int textWidth = 0;
        int lineTextWidth = 0;
        // Precalc the width
        int usedSpacing = 0;
        for (uint character : text)
        {
            if (character == '\n')
            {
                textWidth = std::max<int>(textWidth, lineTextWidth - usedSpacing);
                usedSpacing = 0;
                continue;
            }

            charData = fontImpl.getChar(character);
            // Ignore missing characters
            if (!charData)
                continue;

            /* Advance the cursor to the start of the next character */
            lineTextWidth += charData->ax + options.letterSpacing();
            usedSpacing = options.letterSpacing();
        }
        textWidth = std::max<int>(textWidth, lineTextWidth - usedSpacing);
        if (options.hasShadow())
        {
            textWidth += options.shadowX();
        }

        x -= textWidth;

        if (x < 0)
        {
            x = 0;
        }
    }

    const int originX = x;

    int lineStartX = originX;
    int lineEndX = originX;
    int lineBaselineY = y;

    x = originX;
    for (size_t i = 0; i < text.size(); ++i)
    {
        char character = text[i];
        if (character == '\n')
        {
            if (options.underline() && lineEndX != lineStartX)
            {
                underlineSegments.push_back({
                    lineStartX,
                    lineEndX,
                    lineBaselineY - fontImpl.descender() + 1,
                });
            }

            x = originX;
            y += fontImpl.lineHeight() + 2;

            lineStartX = originX;
            lineEndX = originX;
            lineBaselineY = y;
            continue;
        }

        charData = fontImpl.getChar(character);
        // Ignore missing characters
        if (!charData)
            continue;

        int x2 = x + charData->bl;
        int y2 = y - charData->bt;
        int w = charData->bw;
        int h = charData->bh;

        /* Advance the cursor to the start of the next character */
        x += charData->ax + options.letterSpacing();
        y += charData->ay;
        lineEndX = x;

        /* Skip glyphs that have no pixels */
        if (w <= 0 || h <= 0)
            continue;

        // Calculate some common coordinate values
        int x1 = x2 + w;
        int y1 = y2 + h;
        float tu1 = charData->tx;
        float tv1 = charData->ty;
        float tu2 = charData->tx2;
        float tv2 = charData->ty2;

        const auto addVertices
            = [&vertices](uint color, int x1, int x2, int y1, int y2, float tu1, float tu2, float tv1, float tv2) {
            RenIVertex vx;
            vx.color = color;
            vx.z = 0;
            vx.x = x2;
            vx.y = y2;
            vx.tu = tu1;
            vx.tv = tv1;
            vertices.push_back(vx);
            vx.x = x1;
            vx.y = y2;
            vx.tu = tu2;
            vx.tv = tv1;
            vertices.push_back(vx);
            vx.x = x2;
            vx.y = y1;
            vx.tu = tu1;
            vx.tv = tv2;
            vertices.push_back(vx);
            vx.x = x1;
            vx.y = y2;
            vx.tu = tu2;
            vx.tv = tv1;
            vertices.push_back(vx);
            vx.x = x2;
            vx.y = y1;
            vx.tu = tu1;
            vx.tv = tv2;
            vertices.push_back(vx);
            vx.x = x1;
            vx.y = y1;
            vx.tu = tu2;
            vx.tv = tv2;
            vertices.push_back(vx);
        };

        if (options.hasShadow())
        {
            addVertices(
                secondColor,
                x1 + options.shadowX(),
                x2 + options.shadowX(),
                y1 + options.shadowY(),
                y2 + options.shadowY(),
                tu1,
                tu2,
                tv1,
                tv2);
        }

        if (options.hasOutline())
        {
            const int thickness = options.outlineThickness();
            for (int ring = 1; ring <= thickness; ++ring)
            {
                for (int ox = -ring; ox <= ring; ++ox)
                {
                    const int absOx = (ox < 0) ? -ox : ox;
                    for (int oy = -ring; oy <= ring; ++oy)
                    {
                        const int absOy = (oy < 0) ? -oy : oy;
                        const int maxAbs = (absOx > absOy) ? absOx : absOy;
                        if (maxAbs != ring)
                            continue;
                        if (ox == 0 && oy == 0)
                            continue;

                        addVertices(outlineColor, x1 + ox, x2 + ox, y1 + oy, y2 + oy, tu1, tu2, tv1, tv2);
                    }
                }
            }
        }
        addVertices(fontColor, x1, x2, y1, y2, tu1, tu2, tv1, tv2);
    }
    ASSERT(!vertices.empty(), "drawText called with text that produced no glyphs");
    if (vertices.empty())
        return;

    RenDevice::current()->recordCommand(Command::setCullFace(false));
    RenDevice::current()->renderScreenspace(
        &vertices.front(),
        vertices.size(),
        PrimitiveTopology::Triangles,
        target_.width(),
        target_.height(),
        fontImpl.textureId);

    if (options.underline() && lineEndX != lineStartX)
    {
        underlineSegments.push_back({
            lineStartX,
            lineEndX,
            lineBaselineY - fontImpl.descender() + 1,
        });
    }

    for (const UnderlineSegment& seg : underlineSegments)
    {
        line(Point(seg.x1, seg.y), Point(seg.x2, seg.y), options.color(), 1);
    }
}

void Painter::blit(
    const RenSurface& source, const std::optional<Rect>& srcArea, Point dest, BlitMode mode) const
{
    target_.simpleBlit(source, srcArea, dest, mode);
}

void Painter::tileBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea) const
{
    target_.tileBlit(source, srcArea, destArea);
}

void Painter::stretchBlit(
    const RenSurface& source, const Rect& srcArea, const Rect& destArea, BlitMode mode) const
{
    target_.stretchBlit(source, srcArea, destArea, mode);
}

void Painter::stretchBlit(const RenSurface& source, BlitMode mode) const
{
    target_.stretchBlit(source, mode);
}

void Painter::blitInRequestedSize(const RenSurface& source, Point dest, BlitMode mode) const
{
    target_.blitInRequestedSize(source, dest, mode);
}

} // namespace Ren
