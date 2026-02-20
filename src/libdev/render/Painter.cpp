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

    if (target_.internals())
        target_.internals()->filledRectangle(area, packColour(colour.r(), colour.g(), colour.b(), colour.a()));
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
    if (target_.internals() && target_.internals()->isOffscreen())
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
    if (target_.internals() && target_.internals()->isOffscreen())
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
    target_.internals()->drawText(x, y, text, font, options);
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
