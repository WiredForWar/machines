/*
 * C T X S C E N R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxscenr.hpp"

#include "base/diag.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/dbsystem.hpp"
#include "machgui/dbplanet.hpp"
#include "machgui/dbscenar.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/dbplyscn.hpp"
#include "machgui/database.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/dbtexdat.hpp"
#include "machgui/ui/ScrollableText.hpp"
#include "machgui/ui/VerticalScrollBar.hpp"
#include "machgui/gui.hpp"
#include "gui/font.hpp"
#include "gui/restring.hpp"
#include "gui/painter.hpp"
#include "gui/event.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/image.hpp"
#include "ani/AniSmacker.hpp"
#include "ani/AniSmackerRegular.hpp"
#include "system/registry.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "device/cd.hpp"
#include "machgui/menus_helper.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiSystemPlanetScenarioListBox : public MachGuiSingleSelectionListBox
{
public:
    MachGuiSystemPlanetScenarioListBox(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc)
        : MachGuiSingleSelectionListBox(pParent, pParent, box, horizontalSpacing, verticalSpacing, scrollInc)
    {
    }

    void doNavSelectNewItem(MachGuiSingleSelectionListBoxItem* pItem) override;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiSystemPlanetScenarioListBoxItem : public MachGuiSingleSelectionListBoxItem
{
public:
    MachGuiSystemPlanetScenarioListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        const string& text)
        : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, text)
    {
    }

    virtual void updateDisplayInfo() = 0;

    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override
    {
        MachGuiSingleSelectionListBoxItem::doHandleMouseClickEvent(rel);

        if (rel.leftButton() == Gui::PRESSED)
        {
            updateDisplayInfo();
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiSystemPlanetScenarioListBox::doNavSelectNewItem(MachGuiSingleSelectionListBoxItem* pItem)
{
    pItem->selectThisItem();

    // Make sure smacker animation and text is updated
    MachGuiSystemPlanetScenarioListBoxItem* pRCItem = _REINTERPRET_CAST(MachGuiSystemPlanetScenarioListBoxItem*, pItem);
    pRCItem->updateDisplayInfo();
}

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiPlanetListBoxItem : public MachGuiSystemPlanetScenarioListBoxItem
// Canonical form revoked
{
public:
    MachGuiPlanetListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        MachGuiDbPlanet& planet,
        MachGuiCtxScenario* pScenarioCtx)
        : MachGuiSystemPlanetScenarioListBoxItem(pStartupScreens, pListBox, width, displayString(planet))
        , planet_(planet)
        , pScenarioCtx_(pScenarioCtx)
    {
    }

    ~MachGuiPlanetListBoxItem() override { }

    static bool allScenariosCompleted(MachGuiDbPlanet& planet)
    {
        // Work out if all the scenarios on this planet have been played and won
        // Iterate through scenarios
        uint numScenarios = planet.nScenarios();

        bool allComplete = true;

        for (uint loop = 0; loop < numScenarios && allComplete; ++loop)
        {
            MachGuiDbScenario& scenario = planet.scenario(loop);

            MachGuiDbPlayerScenario* pPlayerScenario = nullptr;

            if (MachGuiDatabase::instance().hasCurrentPlayer()
                && MachGuiDatabase::instance().currentPlayer().hasPlayed(scenario, &pPlayerScenario)
                && pPlayerScenario->hasWon())
            {
                // Level completed
            }
            else
            {
                allComplete = false;
            }
        }

        return allComplete;
    }

protected:
    void select() override
    {
        MachGuiSingleSelectionListBoxItem::select();

        pScenarioCtx_->updateScenarioList(planet_);
    }

    void updateDisplayInfo() override
    {
        pScenarioCtx_->updateDisplayedInfo(planet_.textData().campaignText(), SysPathName(planet_.campaignPicture()));
    }

    static string displayString(MachGuiDbPlanet& planet)
    {
        string retValue;

        if (allScenariosCompleted(planet))
        {
            retValue = MachGui::wonSymbolText();
        }

        retValue += planet.menuString();

        return retValue;
    }

private:
    MachGuiPlanetListBoxItem(const MachGuiPlanetListBoxItem&);
    MachGuiPlanetListBoxItem& operator=(const MachGuiPlanetListBoxItem&);

    // Data members...
    MachGuiDbPlanet& planet_;
    MachGuiCtxScenario* pScenarioCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiSystemListBoxItem : public MachGuiSystemPlanetScenarioListBoxItem
// Canonical form revoked
{
public:
    MachGuiSystemListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        MachGuiDbSystem& system,
        MachGuiCtxScenario* pScenarioCtx)
        : MachGuiSystemPlanetScenarioListBoxItem(pStartupScreens, pListBox, width, displayString(system))
        , system_(system)
        , pScenarioCtx_(pScenarioCtx)
    {
    }

    ~MachGuiSystemListBoxItem() override { }

protected:
    void select() override
    {
        MachGuiSingleSelectionListBoxItem::select();

        pScenarioCtx_->updatePlanetList(system_);
    }

    void updateDisplayInfo() override
    {
        pScenarioCtx_->updateDisplayedInfo(system_.textData().campaignText(), SysPathName(system_.campaignPicture()));
    }

    static string displayString(MachGuiDbSystem& system)
    {
        string retValue;

        if (allPlanetsCompleted(system))
        {
            retValue = MachGui::wonSymbolText();
        }

        retValue += system.menuString();

        return retValue;
    }

private:
    MachGuiSystemListBoxItem(const MachGuiSystemListBoxItem&);
    MachGuiSystemListBoxItem& operator=(const MachGuiSystemListBoxItem&);

    // Work out if all the planets in this system have been played and won
    static bool allPlanetsCompleted(MachGuiDbSystem& system)
    {
        uint numPlanets = system.nPlanets();

        bool allComplete = true;

        for (uint loop = 0; loop < numPlanets && allComplete; ++loop)
        {
            MachGuiDbPlanet& planet = system.planet(loop);

            allComplete = MachGuiPlanetListBoxItem::allScenariosCompleted(planet);
        }

        return allComplete;
    }

    // Data members...
    MachGuiDbSystem& system_;
    MachGuiCtxScenario* pScenarioCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiScenarioListBoxItem : public MachGuiSystemPlanetScenarioListBoxItem
// Canonical form revoked
{
public:
    MachGuiScenarioListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        MachGuiDbScenario& scenario,
        MachGuiCtxScenario* pScenarioCtx)
        : MachGuiSystemPlanetScenarioListBoxItem(pStartupScreens, pListBox, width, displayString(scenario))
        , scenario_(scenario)
        , pScenarioCtx_(pScenarioCtx)
    {
    }

    ~MachGuiScenarioListBoxItem() override { }

protected:
    static string displayString(MachGuiDbScenario& scenario)
    {
        string retValue;
        MachGuiDbPlayerScenario* pPlayerScenario = nullptr;

        if (MachGuiDatabase::instance().hasCurrentPlayer()
            && MachGuiDatabase::instance().currentPlayer().hasPlayed(scenario, &pPlayerScenario))
        {
            ASSERT(pPlayerScenario, "failed to get player scenario information");

            if (pPlayerScenario->hasWon())
            {
                retValue = MachGui::wonSymbolText();
            }
        }

        retValue += scenario.menuString();

        return retValue;
    }

    void select() override
    {
        MachGuiSingleSelectionListBoxItem::select();

        pScenarioCtx_->updateSelectedScenario(scenario_);
    }

    void updateDisplayInfo() override
    {
        pScenarioCtx_->updateDisplayedInfo(
            scenario_.textData().campaignText(),
            SysPathName(scenario_.campaignPicture()));
    }

private:
    MachGuiScenarioListBoxItem(const MachGuiScenarioListBoxItem&);
    MachGuiScenarioListBoxItem& operator=(const MachGuiScenarioListBoxItem&);

    // Data members...
    MachGuiDbScenario& scenario_;
    MachGuiCtxScenario* pScenarioCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

MachGuiCtxScenario::MachGuiCtxScenario(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sl", pStartupScreens)
{
    const int LB_MINX = MachGui::menuScaleFactor() * 38;
    const int LB_MAXX = MachGui::menuScaleFactor() * 251;
    const int SYSTEM_LB_MINY = MachGui::menuScaleFactor() * 27;
    const int SYSTEM_LB_MAXY = MachGui::menuScaleFactor() * 120;
    const int PLANET_LB_MINY = MachGui::menuScaleFactor() * 140;
    const int PLANET_LB_MAXY = MachGui::menuScaleFactor() * 254;
    const int SCENARIO_LB_MINY = MachGui::menuScaleFactor() * 274;
    const int SCENARIO_LB_MAXY = MachGui::menuScaleFactor() * 453;

    const auto& topLeft = getBackdropTopLeft();

    // Regular menu buttons...
    MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(362, 305, 532, 343) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_OK,
        MachGui::ButtonEvent::OK);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(362, 400, 532, 438) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);

    pCancelBtn->escapeControl(true);
    pOkBtn->defaultControl(true);

    // Display system list box heading
    GuiResourceString systemHeading(IDS_MENULB_SYSTEM);
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::largeFontLight()));
    MachGuiMenuText* pSystemText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            LB_MINX,
            SYSTEM_LB_MINY,
            LB_MINX + font.textWidth(systemHeading.asString()),
            SYSTEM_LB_MINY + font.charHeight() + 2),
        IDS_MENULB_SYSTEM,
        MachGui::Menu::largeFontLight());

    // Display planet list box heading
    GuiResourceString planetHeading(IDS_MENULB_PLANET);
    MachGuiMenuText* pPlanetText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            LB_MINX,
            PLANET_LB_MINY,
            LB_MINX + font.textWidth(planetHeading.asString()),
            PLANET_LB_MINY + font.charHeight() + 2),
        IDS_MENULB_PLANET,
        MachGui::Menu::largeFontLight());

    // Display scenario list box heading
    GuiResourceString scenarioHeading(IDS_MENULB_SCENARIO);
    MachGuiMenuText* pScenarioText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            LB_MINX,
            SCENARIO_LB_MINY,
            LB_MINX + font.textWidth(scenarioHeading.asString()),
            SCENARIO_LB_MINY + font.charHeight() + 2),
        IDS_MENULB_SCENARIO,
        MachGui::Menu::largeFontLight());

    // Create system list box
    pSystemList_ = new MachGuiSystemPlanetScenarioListBox(
        pStartupScreens,
        Gui::Box(LB_MINX, pSystemText->absoluteBoundary().maxCorner().y() - topLeft.first, LB_MAXX, SYSTEM_LB_MAXY),
        1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);
    // Create planet list box
    pPlanetList_ = new MachGuiSystemPlanetScenarioListBox(
        pStartupScreens,
        Gui::Box(LB_MINX, pPlanetText->absoluteBoundary().maxCorner().y() - topLeft.first, LB_MAXX, PLANET_LB_MAXY),
        1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);
    // Create scenario list box
    pScenarioList_ = new MachGuiSystemPlanetScenarioListBox(
        pStartupScreens,
        Gui::Box(LB_MINX, pScenarioText->absoluteBoundary().maxCorner().y() - topLeft.first, LB_MAXX, SCENARIO_LB_MAXY),
        1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);

    pTextInfo_ = new MachGuiScrollableText(pStartupScreens, Gui::Box(338, 155, 556, 260) * MachGui::menuScaleFactor());
    MachGuiVerticalScrollBar::createWholeBar(
        pStartupScreens,
        Gui::Coord(557, 155) * MachGui::menuScaleFactor(),
        104 * MachGui::menuScaleFactor(),
        pTextInfo_);

#ifdef DEMO
    if (!MachGuiDatabase::instance().nPlayers())
    {
        MachGuiDbPlayer& newPlayer = MachGuiDatabase::instance().addPlayer("Dummy");
        MachGuiDatabase::instance().currentPlayer(&newPlayer);
        MachGuiDatabase::instance().writeDatabase();
    }
    else
    {
        MachGuiDbPlayer& player = MachGuiDatabase::instance().player(0);
        MachGuiDatabase::instance().currentPlayer(&player);
    }
#endif

    workOutWhichScenarioToDefaultTo();
    updateSystemList();

    TEST_INVARIANT;
}

MachGuiCtxScenario::~MachGuiCtxScenario()
{

    TEST_INVARIANT;
}

void MachGuiCtxScenario::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxScenario& t)
{

    o << "MachGuiCtxScenario " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxScenario " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxScenario::update()
{
    animations_.update();
}

void MachGuiCtxScenario::updateSystemList()
{
    const int LB_MINX = MachGui::menuScaleFactor() * 38;
    const int LB_MAXX = MachGui::menuScaleFactor() * 251;

    uint numSystems = MachGuiDatabase::instance().nCampaignSystems();
    bool selectFirstItem = true;
    MachGuiSystemListBoxItem* pFirstItem = nullptr;
    MachGuiDbSystem* pFirstSystem = nullptr;

    for (uint loop = 0; loop < numSystems; ++loop)
    {
        MachGuiDbSystem& system = MachGuiDatabase::instance().campaignSystem(loop);

        if (system.isAvailable()
#ifndef PRODUCTION
            || getenv("cb_showallscenarios")
#endif
        )
        {
            MachGuiSystemListBoxItem* pItem
                = new MachGuiSystemListBoxItem(pStartupScreens_, pSystemList_, LB_MAXX - LB_MINX, system, this);
            // Store first item in list
            if (! pFirstItem)
            {
                pFirstItem = pItem;
                pFirstSystem = &system;
            }

            // See if this is the next system to be played
            if (pDefaultScenario_ && &pDefaultScenario_->planet().system() == &system)
            {
                pItem->selectThisItem();
                selectFirstItem = false;
                // Display info about this system, planet or scenario
                if (pDefaultScenario_ == &pDefaultScenario_->planet().scenario(0))
                {
                    if (&pDefaultScenario_->planet() == &pDefaultScenario_->planet().system().planet(0))
                    {
                        // Display system information
                        updateDisplayedInfo(system.textData().campaignText(), SysPathName(system.campaignPicture()));
                    }
                    else
                    {
                        // Display planet information
                        updateDisplayedInfo(
                            pDefaultScenario_->planet().textData().campaignText(),
                            SysPathName(pDefaultScenario_->planet().campaignPicture()));
                    }
                }
                else
                {
                    // Display scenario information
                    updateDisplayedInfo(
                        pDefaultScenario_->textData().campaignText(),
                        SysPathName(pDefaultScenario_->campaignPicture()));
                }
            }
        }
    }

    if (selectFirstItem)
    {
        ASSERT(pFirstItem, "no systems in list box");
        ASSERT(pFirstSystem, "failed to store pointer to system");

        pFirstItem->selectThisItem();
        // Display info about this system
        updateDisplayedInfo(pFirstSystem->textData().campaignText(), SysPathName(pFirstSystem->campaignPicture()));
    }

    pSystemList_->childrenUpdated();
}

void MachGuiCtxScenario::updatePlanetList(MachGuiDbSystem& system)
{
    const int LB_MINX = MachGui::menuScaleFactor() * 38;
    const int LB_MAXX = MachGui::menuScaleFactor() * 251;

    // Delete all items from list
    pPlanetList_->deleteAllItems();

    // Redraw backdrop to list
    auto* shared = pStartupScreens_->getSharedBitmaps();
    auto backdrop = shared->getNamedBitmap("backdrop");
    shared->blitNamedBitmapFromArea(
        backdrop,
        pPlanetList_->absoluteBoundary(),
        pPlanetList_->absoluteBoundary().minCorner(),
        [shared, backdrop](const Gui::Box& box) {
            using namespace machgui::helper::menus;
            return centered_bitmap_transform(
                box,
                shared->getWidthOfNamedBitmap(backdrop),
                shared->getHeightOfNamedBitmap(backdrop));
        });

    // Insert new items into list
    uint numPlanets = system.nPlanets();
    bool selectFirstItem = true;
    MachGuiPlanetListBoxItem* pFirstItem = nullptr;

    for (uint loop = 0; loop < numPlanets; ++loop)
    {
        MachGuiDbPlanet& planet = system.planet(loop);

        if (planet.isAvailable()
#ifndef PRODUCTION
            || getenv("cb_showallscenarios")
#endif
        )
        {
            MachGuiPlanetListBoxItem* pItem
                = new MachGuiPlanetListBoxItem(pStartupScreens_, pPlanetList_, LB_MAXX - LB_MINX, planet, this);

            // Store first item in list
            if (! pFirstItem)
            {
                pFirstItem = pItem;
            }

            // See if this is the next planet to be played
            if (pDefaultScenario_ && &pDefaultScenario_->planet() == &planet)
            {
                pItem->selectThisItem();
                selectFirstItem = false;
            }
        }
    }

    if (selectFirstItem)
    {
        ASSERT(pFirstItem, "no planets in list box");

        pFirstItem->selectThisItem();
    }

    pPlanetList_->childrenUpdated();
}

void MachGuiCtxScenario::updateScenarioList(MachGuiDbPlanet& planet)
{
    const int LB_MINX = MachGui::menuScaleFactor() * 38;
    const int LB_MAXX = MachGui::menuScaleFactor() * 251;

    // Delete all items from list
    pScenarioList_->deleteAllItems();

    // Redraw backdrop to list
    auto* shared = pStartupScreens_->getSharedBitmaps();
    auto backdrop = shared->getNamedBitmap("backdrop");
    shared->blitNamedBitmapFromArea(
        backdrop,
        pScenarioList_->absoluteBoundary(),
        pScenarioList_->absoluteBoundary().minCorner(),
        [shared, backdrop](const Gui::Box& box) {
            using namespace machgui::helper::menus;
            return centered_bitmap_transform(
                box,
                shared->getWidthOfNamedBitmap(backdrop),
                shared->getHeightOfNamedBitmap(backdrop));
        });

    // Insert new items into list
    uint numScenarios = planet.nScenarios();
    bool selectFirstItem = true;
    MachGuiScenarioListBoxItem* pFirstItem = nullptr;

    for (uint loop = 0; loop < numScenarios; ++loop)
    {
        MachGuiDbScenario& scenario = planet.scenario(loop);

        if (scenario.isAvailable()
#ifndef PRODUCTION
            || getenv("cb_showallscenarios")
#endif
        )
        {
            MachGuiScenarioListBoxItem* pItem
                = new MachGuiScenarioListBoxItem(pStartupScreens_, pScenarioList_, LB_MAXX - LB_MINX, scenario, this);

            // Store first item in list
            if (! pFirstItem)
            {
                pFirstItem = pItem;
            }

            // See if this is the next scenario to be played
            if (pDefaultScenario_ == &scenario)
            {
                pItem->selectThisItem();
                selectFirstItem = false;
            }
        }
    }

    if (selectFirstItem)
    {
        ASSERT(pFirstItem, "no scenarios in list box");

        pFirstItem->selectThisItem();
    }

    pScenarioList_->childrenUpdated();
}

void MachGuiCtxScenario::updateSelectedScenario(MachGuiDbScenario& scenario)
{
    PRE(MachGuiDatabase::instance().hasCurrentPlayer());

    pStartupScreens_->startupData()->scenario(&scenario);

    MachGuiDatabase::instance().currentPlayer().lastSelectedScenario(&scenario);
}

void MachGuiCtxScenario::updateDisplayedInfo(const string& text, SysPathName animation)
{
    if (pImage_)
    {
        delete pImage_;
        pImage_ = nullptr;
    }
    pStartupScreens_->clearAllSmackerAnimations();
    WAYNE_STREAM("MachGuiCtxScenario::updateSelectedScenario animation filename: " << animation << std::endl);

    // Get flic off hard-disk or CD-Rom
    if (! animation.existsAsFile())
    {
        // Make sure the cd is stopped before accessing files on it.
        if (DevCD::instance().isPlayingAudioCd())
        {
            DevCD::instance().stopPlaying();
        }

        string cdRomDrive;

        if (MachGui::getCDRomDriveContainingFile(cdRomDrive, animation.c_str()))
        {
            animation = SysPathName(cdRomDrive + animation.c_str());

            // Can't play music and smacker anim off CD at same time
            if (animation.existsAsFile())
            {
                pStartupScreens_->desiredCdTrack(MachGuiStartupScreens::DONT_PLAY_CD);
            }
        }
        else
        {
            animation = SysPathName("gui/menu/cds.smk");
        }
    }

    if (animation.existsAsFile())
    {
        const Gui::Coord animationCoord = Gui::Coord(342, 32) * MachGui::menuScaleFactor();
        if (animation.extension() == "smk")
        {
            // File is smacker file
            const auto& topLeft = getBackdropTopLeft();
            AniSmacker* pSmackerAnimation = new AniSmackerRegular(
                animation,
                animationCoord.x() + topLeft.second,
                animationCoord.y() + topLeft.first);
            pSmackerAnimation->setScaleFactor(MachGui::menuScaleFactor());
            pStartupScreens_->addSmackerAnimation(pSmackerAnimation);
        }
        else if (animation.extension() == "bmp")
        {
            // File is a bitmap
            pImage_ = new GuiImage(
                pStartupScreens_,
                animationCoord,
                MachGui::getScaledImage(animation.pathname(), MachGui::menuScaleFactor()));
        }
    }
    pStartupScreens_->playSmackerAnimations();
    pTextInfo_->setText(text);
}

void MachGuiCtxScenario::workOutWhichScenarioToDefaultTo()
{
    PRE(MachGuiDatabase::instance().hasCurrentPlayer());

    pDefaultScenario_ = MachGuiDatabase::instance().currentPlayer().lastSelectedScenario();
}

// virtual
bool MachGuiCtxScenario::okayToSwitchContext()
{
    MachGuiDatabase::instance().writeDatabase();

    return true;
}

/* End CTXSCENR.CPP *************************************************/
