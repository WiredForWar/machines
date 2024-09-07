/*
 * C T X S A V E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxsave.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/dbsavgam.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/database.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/editlist.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/msgbox.hpp"
#include "machgui/ingame.hpp"
#include "machgui/lsgextra.hpp"
#include "machgui/ui/VerticalScrollBar.hpp"
#include "machlog/races.hpp"
#include "system/pathname.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/font.hpp"
#include "gui/restring.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include <stdio.h>

#include "spdlog/spdlog.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiSaveGameListBoxItem : public MachGuiSingleSelectionListBoxItem
// Canonical form revoked
{
public:
    MachGuiSaveGameListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        MachGuiDbSavedGame& savedGame,
        MachGuiCtxSave* pSaveCtx)
        : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, displayText(savedGame))
        , savedGame_(savedGame)
        , pSaveCtx_(pSaveCtx)
    {
        disabled_ = false;

        if (savedGame.hasPlayer() && MachGuiDatabase::instance().hasCurrentPlayer())
        {
            MachGuiDbPlayer* pPlayer1 = &savedGame.player();
            MachGuiDbPlayer* pPlayer2 = &MachGuiDatabase::instance().currentPlayer();

            if (pPlayer1 != pPlayer2)
            {
                disabled_ = true;
            }
        }
    }

    ~MachGuiSaveGameListBoxItem() override { }

    static string displayText(MachGuiDbSavedGame& savedGame)
    {
        string retVal;
        retVal = savedGame.userFileName();

        if (savedGame.hasPlayer())
        {
            MachGuiDbPlayer& player = savedGame.player();
            retVal += " (";
            retVal += player.name();
            retVal += ")";
        }
        return retVal;
    }

    void CLASS_INVARIANT;

protected:
    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override
    {
        if (! disabled_)
        {
            MachGuiSingleSelectionListBoxItem::doHandleMouseEnterEvent(rel);
        }
    }

    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override
    {
        if (! disabled_)
        {
            MachGuiSingleSelectionListBoxItem::doHandleMouseExitEvent(rel);
        }
    }

    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override
    {
        if (! disabled_)
        {
            MachGuiSingleSelectionListBoxItem::doHandleMouseClickEvent(rel);
        }
    }

    void select() override
    {
        MachGuiSingleSelectionListBoxItem::select();

        pSaveCtx_->selectedSaveGame(&savedGame_);
    }

    void unselect() override
    {
        MachGuiSingleSelectionListBoxItem::unselect();

        pSaveCtx_->clearSelectedSaveGame();
    }

private:
    MachGuiSaveGameListBoxItem(const MachGuiSaveGameListBoxItem&);
    MachGuiSaveGameListBoxItem& operator=(const MachGuiSaveGameListBoxItem&);

    // Data members...
    MachGuiDbSavedGame& savedGame_;
    MachGuiCtxSave* pSaveCtx_;
    bool disabled_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiSaveGameDeleteMessageBoxResponder : public MachGuiMessageBoxResponder
{
public:
    MachGuiSaveGameDeleteMessageBoxResponder(MachGuiCtxSave* pSaveCtx)
        : pSaveCtx_(pSaveCtx)
    {
    }

    bool okPressed() override
    {
        pSaveCtx_->deleteSavedGame();
        return true;
    }

private:
    MachGuiCtxSave* pSaveCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

#define SAVE_LB_MINX 98
#define SAVE_LB_MAXX 312
#define SAVE_LB_MINY 72
#define SAVE_LB_MAXY 412
#define SCROLLBAR_WIDTH 17

MachGuiCtxSave::MachGuiCtxSave(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sh", pStartupScreens)
{
    const auto& topLeft = getBackdropTopLeft();

    // Standard buttons...
    pOkBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(387, 111, 578, 154) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_OK,
        MachGui::ButtonEvent::OK);
    pDeleteBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(387, 215, 578, 258) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_DELETE,
        MachGui::ButtonEvent::DELETE);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(387, 312, 578, 356) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);

    pCancelBtn->escapeControl(true);
    pOkBtn_->defaultControl(true);

    // Display save list box heading
    GuiResourceString saveHeading(IDS_MENULB_SAVEGAME);
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::largeFontLight()));
    MachGuiMenuText* pSaveText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            MachGui::menuScaleFactor() * SAVE_LB_MINX,
            MachGui::menuScaleFactor() * SAVE_LB_MINY,
            MachGui::menuScaleFactor() * SAVE_LB_MINX + font.textWidth(saveHeading.asString()),
            MachGui::menuScaleFactor() * SAVE_LB_MINY + font.charHeight() + MachGui::menuScaleFactor() * 2),
        IDS_MENULB_SAVEGAME,
        MachGui::Menu::largeFontLight());

    // Create save game list box
    pSaveGameList_ = new MachGuiSingleSelectionListBox(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(
            MachGui::menuScaleFactor() * SAVE_LB_MINX,
            pSaveText->absoluteBoundary().maxCorner().y() - topLeft.first,
            MachGui::menuScaleFactor() * (SAVE_LB_MAXX - SCROLLBAR_WIDTH),
            MachGui::menuScaleFactor() * SAVE_LB_MAXY),
        MachGui::menuScaleFactor() * 1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1 * MachGui::menuScaleFactor());

    const int barHeight = MachGui::menuScaleFactor() * (SAVE_LB_MAXY - SAVE_LB_MINY)
        - (pSaveText->absoluteBoundary().maxCorner().y() - pSaveText->absoluteBoundary().minCorner().y());
    MachGuiVerticalScrollBar::createWholeBar(
        pStartupScreens,
        Gui::Coord(
            MachGui::menuScaleFactor() * (SAVE_LB_MAXX - SCROLLBAR_WIDTH),
            pSaveText->absoluteBoundary().maxCorner().y() - topLeft.first),
        barHeight,
        pSaveGameList_);

    updateSaveGameList();

    TEST_INVARIANT;
}

MachGuiCtxSave::~MachGuiCtxSave()
{
    TEST_INVARIANT;
}

void MachGuiCtxSave::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxSave& t)
{

    o << "MachGuiCtxSave " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxSave " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxSave::update()
{
    animations_.update();
    pNewSaveGameName_->update();
}

void MachGuiCtxSave::updateSaveGameList()
{
    clearSelectedSaveGame();
    pSaveGameList_->deleteAllItems();

    // Create special [new save game] entry in list box.
    const int itemWidth = (SAVE_LB_MAXX - SAVE_LB_MINX - SCROLLBAR_WIDTH) * MachGui::menuScaleFactor();
    GuiResourceString newSaveGameStr(IDS_MENU_NEWSAVEGAME);
    pNewSaveGameName_ = new MachGuiEditBoxListBoxItem(
        pStartupScreens_,
        pSaveGameList_,
        itemWidth,
        newSaveGameStr.asString());
    pNewSaveGameName_->maxChars(MAX_SAVEGAMENAME_LEN);

    // Add previously saved games into list box.
    uint numSavedGames = MachGuiDatabase::instance().nSavedGames();

    for (uint loop = 0; loop < numSavedGames; ++loop)
    {
        MachGuiDbSavedGame& savedGame = MachGuiDatabase::instance().savedGame(loop);
        new MachGuiSaveGameListBoxItem(
            pStartupScreens_,
            pSaveGameList_,
            itemWidth,
            savedGame,
            this);
    }

    // Get list to redraw.
    pSaveGameList_->childrenUpdated();
}

// virtual
bool MachGuiCtxSave::okayToSwitchContext()
{
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::OK)
    {
        // Create new save game...
        if (pNewSaveGameName_->isSelected())
        {
            if (pNewSaveGameName_->text() != "")
            {
                // Check for unique saved game name
                bool uniqueSaveGameName = true;
                uint numSavedGames = MachGuiDatabase::instance().nSavedGames();

                for (uint loop = 0; loop < numSavedGames; ++loop)
                {
                    MachGuiDbSavedGame& savedGame = MachGuiDatabase::instance().savedGame(loop);
                    if (strcasecmp(savedGame.userFileName().c_str(), pNewSaveGameName_->text().c_str()) == 0)
                    {
                        uniqueSaveGameName = false;
                    }
                }

                if (uniqueSaveGameName)
                {
                    // Save the game, the name is unique so we can save it...
                    bool saveSuccess = saveGame(pNewSaveGameName_->text());

                    if (! saveSuccess)
                    {
                        // Display error message, game failed to save...
                        pStartupScreens_->displayMsgBox(IDS_MENUMSG_SAVEFAIL);
                    }

                    return saveSuccess;
                }
                else
                {
                    // Display error message, name is not unique...
                    pStartupScreens_->displayMsgBox(IDS_MENUMSG_SAVEDGAMENAMEEXISTS);
                    return false;
                }
            }
            else
            {
                pStartupScreens_->displayMsgBox(IDS_MENUMSG_ENTERSAVEGAMENAME);
                return false;
            }
        }
        else // Select previously saved game ( save over )...
        {
            if (! pSelectedSaveGame_)
            {
                pStartupScreens_->displayMsgBox(IDS_MENUMSG_ENTERSAVEGAMENAME);
                return false;
            }
            else
            {
                string saveDisplayName = pSelectedSaveGame_->userFileName();
                deleteSavedGame();
                saveGame(saveDisplayName);
                return true;
            }
        }
    }

    return true;
}

// virtual
void MachGuiCtxSave::buttonEvent(MachGui::ButtonEvent be)
{
    if (be == MachGui::ButtonEvent::DELETE)
    {
        if (pSelectedSaveGame_)
        {
            pStartupScreens_->displayMsgBox(
                IDS_MENUMSG_DELETESAVEDGAME,
                new MachGuiSaveGameDeleteMessageBoxResponder(this));
        }
    }
}

void MachGuiCtxSave::selectedSaveGame(MachGuiDbSavedGame* pSavedGame)
{
    pSelectedSaveGame_ = pSavedGame;
    pDeleteBtn_->disabled(false);
}

void MachGuiCtxSave::clearSelectedSaveGame()
{
    // Disable delete button, shift focus if necessary
    pSelectedSaveGame_ = nullptr;
    if (pDeleteBtn_->hasFocusSet())
    {
        pOkBtn_->hasFocus(true);
        pDeleteBtn_->hasFocus(false);
    }

    pDeleteBtn_->disabled(true);
}

void MachGuiCtxSave::deleteSavedGame()
{
    PRE(pSelectedSaveGame_);

    remove(pSelectedSaveGame_->externalFileName().c_str());
    MachGuiDatabase::instance().removeSavedGame(pSelectedSaveGame_);
    MachGuiDatabase::instance().writeDatabase();
    updateSaveGameList();
}

bool MachGuiCtxSave::saveGame(const string& saveDisplayName)
{
    // Display saving bmp
    GuiBitmap savingBmp = Gui::requestScaledImage("gui/menu/saving.bmp", MachGui::menuScaleFactor());
    savingBmp.enableColourKeying();
    GuiBitmap frontBuffer = W4dManager::instance().sceneManager()->pDevice()->frontSurface();
    const auto& topLeft = getBackdropTopLeft();
    const Gui::Coord coord(topLeft.second, topLeft.first);
    if (savingBmp.requestedSize().isNull())
    {
        GuiPainter::instance().blit(savingBmp, coord);
    }
    else
    {
        Gui::Size backgroundSize(savingBmp.requestedSize().width, savingBmp.requestedSize().height);
        GuiPainter::instance().stretch(savingBmp, Gui::Box(coord, backgroundSize));
    }
    W4dManager::instance().sceneManager()->pDevice()->display()->flipBuffers();

    // Create next filename...
    bool gotSavePathName = false;
    SysPathName savePathName;
    size_t count = 0;

    while (! gotSavePathName)
    {
        char buffer[20];

        sprintf(buffer, "%04ld", count);

        // savePathName = string( "savegame/save" ) + buffer + ".sav";
        savePathName = SysPathName(string("savegame/save") + buffer + ".sav");

        if (! savePathName.existsAsFile())
            gotSavePathName = true;

        ++count;
    }

    // Save game...
    MachGuiLoadSaveGameExtras lsgExtras(&pStartupScreens_->inGameScreen());

    spdlog::info("Saving the game state to file {}", savePathName.c_str());
    bool saveSuccess = MachLogRaces::instance().saveGame(savePathName, &lsgExtras);

    if (saveSuccess)
    {
        // Store reference to save file in database file...
        MachGuiDbSavedGame* pNewSaveGame = new 
            MachGuiDbSavedGame(saveDisplayName, savePathName.c_str(), pStartupScreens_->startupData()->scenario());

        if (MachGuiDatabase::instance().hasCurrentPlayer())
        {
            pNewSaveGame->player(&MachGuiDatabase::instance().currentPlayer());
        }

        if (pStartupScreens_->gameType() == MachGuiStartupScreens::CAMPAIGNGAME)
        {
            pNewSaveGame->isCampaignGame(true);
        }

        MachGuiDatabase::instance().addSavedGame(pNewSaveGame);
        MachGuiDatabase::instance().writeDatabase();
    }

    return saveSuccess;
}

/* End CTXSAVE.CPP **************************************************/
