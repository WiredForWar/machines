/*
 * P A U S E D I M A G E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "machgui/PausedImage.hpp"

#include "gui/gui.hpp"
#include "gui/painter.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/db/Database.hpp"
#include "machgui/db/DbScenario.hpp"
#include "render/device.hpp"
#include "sim/manager.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"

MachGuiPausedImage::MachGuiPausedImage(GuiDisplayable* pParent, MachInGameScreen* pInGameScreen)
    : GuiDisplayable(pParent, {}, GuiDisplayable::LAYER5)
    , pInGameScreen_(pInGameScreen)
{
}

void MachGuiPausedImage::doDisplay()
{
    if (SimManager::instance().isSuspended())
    {
        GuiBitmap pausedBmp;

        switch (pInGameScreen_->actualGameState())
        {
            case MachInGameScreen::WON:
                if (MachGuiDatabase::instance().currentScenario().isTrainingScenario())
                {
                    pausedBmp = Gui::getScaledImage("gui/misc/complete.bmp");
                }
                else
                {
                    pausedBmp = Gui::getScaledImage("gui/misc/victory.bmp");
                }
                break;
            case MachInGameScreen::LOST:
                if (MachGuiDatabase::instance().currentScenario().isTrainingScenario())
                {
                    pausedBmp = Gui::getScaledImage("gui/misc/failed.bmp");
                }
                else
                {
                    pausedBmp = Gui::getScaledImage("gui/misc/defeat.bmp");
                }
                break;
            default:
                pausedBmp = Gui::getScaledImage("gui/misc/paused.bmp");
                break;
        }

        pausedBmp.enableColourKeying();

        Ren::Size windowSize = W4dManager::instance().sceneManager()->pDevice()->windowSize();

        // Centre within the parent's viewport area
        const Gui::Coord topLeft = parent()->absoluteBoundary().minCorner();
        const int xPos = ((windowSize.width - topLeft.x()) / 2) - (pausedBmp.width() / 2);
        const int yPos = ((windowSize.height - topLeft.y()) / 2) - (pausedBmp.height() / 2);

        GuiPainter::instance().blit(pausedBmp, {}, Ren::Point(xPos + topLeft.x(), yPos + topLeft.y()));
    }
    else if (pInGameScreen_->isNetworkStuffed())
    {
        GuiBitmap netBusyBmp = Gui::getScaledImage("gui/misc/netbusy.bmp");
        netBusyBmp.enableColourKeying();

        Ren::Size windowSize = W4dManager::instance().sceneManager()->pDevice()->windowSize();
        const int xPos = windowSize.width - netBusyBmp.width();
        const int borderHeight = windowSize.height * 0.05;

        GuiPainter::instance().blit(netBusyBmp, {}, Ren::Point(xPos, borderHeight + 2 * Gui::uiScaleFactor()));
    }
}
