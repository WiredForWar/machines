/*
 * M A P A R E A . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/maparea.hpp"
#include "machgui/internal/mapareai.hpp"
#include "system/pathname.hpp"
#include "render/device.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"

class MachGuiMapAreaMagic : public GuiDisplayable
{
public:
    MachGuiMapAreaMagic(GuiDisplayable* pParent, const Gui::Box& box, MachGuiMapArea* pMapArea)
        : GuiDisplayable(pParent, box, GuiDisplayable::LAYER1)
        , pMapArea_(pMapArea)
    {
    }

    void doDisplay() override
    {
        // Blit from front to back buffer, this will restore the map area after the
        // 3D rendering has wiped over it
        if (not pMapArea_->hasChanged())
        {
            GuiBitmap frontBuffer = W4dManager::instance().sceneManager()->pDevice()->frontSurface();
            GuiPainter::instance().blit(frontBuffer, absoluteBoundary(), absoluteBoundary().minCorner());
        }

        // Render the underside of the map area.
        static GuiBitmap slideRailBmp = MachGui::getScaledImage("gui/misc/slide.bmp");

        if (not slideRailBmp.isColourKeyingOn())
        {
            slideRailBmp.enableColourKeying();
        }

        GuiPainter::instance().blit(
            slideRailBmp,
            Gui::Coord(absoluteBoundary().minCorner().x(), absoluteBoundary().maxCorner().y() - 1));
    }

    void controlPanelSliding(bool sliding) { redrawEveryFrame(sliding); }

private:
    MachGuiMapArea* pMapArea_;
};

MachGuiMapArea::MachGuiMapArea(GuiDisplayable* pParent, const Gui::Box& box)
    : GuiDisplayable(pParent, box, GuiDisplayable::LAYER2)
    , pImpl_(new MachGuiMapAreaImpl())
{
    CB_DEPIMPL(MachGuiMapAreaMagic*, pMapAreaMagic_);

    pMapAreaMagic_ = new MachGuiMapAreaMagic(this, box, this);

    TEST_INVARIANT;
}

MachGuiMapArea::~MachGuiMapArea()
{
    TEST_INVARIANT;

    delete pImpl_;
}

void MachGuiMapArea::controlPanelSliding(bool sliding)
{
    CB_DEPIMPL(MachGuiMapAreaMagic*, pMapAreaMagic_);

    pMapAreaMagic_->controlPanelSliding(sliding);
}

void MachGuiMapArea::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMapArea& t)
{

    o << "MachGuiMapArea " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMapArea " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiMapArea::doDisplay()
{
    redrawAreaImmediate(Gui::Box(0, 0, width(), height()));
}

void MachGuiMapArea::redrawAreaImmediate(const Gui::Box& area)
{
    int y = 0;
    int maxY = height();
    int yStep = MachGui::controlPanelBmp().height();

    int areaYMax = area.maxCorner().y();
    int areaXMax = area.maxCorner().x();
    int areaYMin = area.minCorner().y();
    int areaXMin = area.minCorner().x();

    while (y < maxY and y < areaYMax)
    {
        if (areaYMin <= y + yStep)
        {
            Gui::Box drawArea(
                std::max(0, areaXMin),
                std::max(y, areaYMin),
                std::min((int)MachGui::controlPanelBmp().width(), areaXMax),
                std::min(y + yStep, areaYMax));

            int drawAreaYMax = drawArea.maxCorner().y();
            int drawAreaXMax = drawArea.maxCorner().x();
            int drawAreaYMin = drawArea.minCorner().y();
            int drawAreaXMin = drawArea.minCorner().x();

            Gui::Coord screenCoords(
                drawAreaXMin + absoluteBoundary().minCorner().x(),
                drawAreaYMin + absoluteBoundary().minCorner().y());

            GuiPainter::instance().blit(
                MachGui::controlPanelBmp(),
                Gui::Box(
                    Gui::Coord(drawAreaXMin, drawAreaYMin - y),
                    drawAreaXMax - drawAreaXMin, /* width */
                    drawAreaYMax - drawAreaYMin /* height*/),
                screenCoords);
        }

        y += MachGui::controlPanelBmp().height();
    }
}

/* End MAPAREA.CPP **************************************************/
