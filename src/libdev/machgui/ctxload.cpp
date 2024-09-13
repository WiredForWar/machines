/*
 * C T X L O A D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxload.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/database.hpp"
#include "machgui/dbsavgam.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/msgbox.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/VerticalScrollBar.hpp"
#include "system/pathname.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/font.hpp"
#include "gui/restring.hpp"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiLoadGameListBoxItem : public MachGuiSingleSelectionListBoxItem
// Canonical form revoked
{
public:
    MachGuiLoadGameListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        MachGuiDbSavedGame& savedGame,
        MachGuiCtxLoad* pLoadCtx)
        : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, displayText(savedGame))
        , savedGame_(savedGame)
        , pLoadCtx_(pLoadCtx)
    {
    }

    ~MachGuiLoadGameListBoxItem() override { }

    static std::string displayText(MachGuiDbSavedGame& savedGame)
    {
        std::string retVal;
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

protected:
    void select() override
    {
        MachGuiSingleSelectionListBoxItem::select();

        pLoadCtx_->selectedSaveGame(&savedGame_);
    }

private:
    MachGuiLoadGameListBoxItem(const MachGuiLoadGameListBoxItem&);
    MachGuiLoadGameListBoxItem& operator=(const MachGuiLoadGameListBoxItem&);

    // Data members...
    MachGuiDbSavedGame& savedGame_;
    MachGuiCtxLoad* pLoadCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiLoadGameDeleteMessageBoxResponder : public MachGuiMessageBoxResponder
{
public:
    MachGuiLoadGameDeleteMessageBoxResponder(MachGuiCtxLoad* pLoadCtx)
        : pLoadCtx_(pLoadCtx)
    {
    }

    bool okPressed() override
    {
        pLoadCtx_->deleteSavedGame();
        return true;
    }

private:
    MachGuiCtxLoad* pLoadCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

#define LOAD_LB_MINX 75
#define LOAD_LB_MAXX 291
#define LOAD_LB_MINY 88
#define LOAD_LB_MAXY 429
#define SCROLLBAR_WIDTH 17

MachGuiCtxLoad::MachGuiCtxLoad(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sg", pStartupScreens)
{
    const auto& topLeft = getBackdropTopLeft();

    // Standard buttons...
    pOkBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(364, 127, 555, 170) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_OK,
        MachGui::ButtonEvent::DUMMY_OK);
    pDeleteBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(364, 230, 555, 274) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_DELETE,
        MachGui::ButtonEvent::DELETE);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(364, 328, 555, 371) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);

    pCancelBtn->escapeControl(true);
    pOkBtn_->defaultControl(true);

    // Display save list box heading
    GuiResourceString loadHeading(IDS_MENULB_LOADGAME);
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::largeFontLight()));
    MachGuiMenuText* pLoadText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            MachGui::menuScaleFactor() * LOAD_LB_MINX,
            MachGui::menuScaleFactor() * LOAD_LB_MINY,
            MachGui::menuScaleFactor() * LOAD_LB_MINX + font.textWidth(loadHeading.asString()),
            MachGui::menuScaleFactor() * LOAD_LB_MINY + font.charHeight() + 2),
        IDS_MENULB_LOADGAME,
        MachGui::Menu::largeFontLight());

    // Create save game list box
    pSaveGameList_ = new MachGuiSingleSelectionListBox(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(
            MachGui::menuScaleFactor() * LOAD_LB_MINX,
            pLoadText->absoluteBoundary().maxCorner().y() - topLeft.first,
            MachGui::menuScaleFactor() * (LOAD_LB_MAXX - SCROLLBAR_WIDTH),
            MachGui::menuScaleFactor() * LOAD_LB_MAXY),
        MachGui::menuScaleFactor() * 1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);

    MachGuiVerticalScrollBar::createWholeBar(
        pStartupScreens,
        Gui::Coord(
            MachGui::menuScaleFactor() * (LOAD_LB_MAXX - SCROLLBAR_WIDTH),
            pLoadText->absoluteBoundary().maxCorner().y() - topLeft.first),
        MachGui::menuScaleFactor() * (LOAD_LB_MAXY - LOAD_LB_MINY)
            - (pLoadText->absoluteBoundary().maxCorner().y() - pLoadText->absoluteBoundary().minCorner().y()),
        pSaveGameList_);

    updateSaveGameList();

    TEST_INVARIANT;
}

MachGuiCtxLoad::~MachGuiCtxLoad()
{
    TEST_INVARIANT;
}

void MachGuiCtxLoad::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxLoad& t)
{

    o << "MachGuiCtxLoad " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxLoad " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxLoad::update()
{
    if (autoLoadGame_)
    {
        pStartupScreens_->buttonAction(MachGui::ButtonEvent::OK);
    }
    else
    {
        animations_.update();
    }
}

void MachGuiCtxLoad::updateSaveGameList()
{
    // Disable delete button, shift focus if necessary
    if (pDeleteBtn_->hasFocusSet())
    {
        pOkBtn_->hasFocus(true);
        pDeleteBtn_->hasFocus(false);
    }
    pDeleteBtn_->disabled(true);

    pSelectedSaveGame_ = nullptr;
    pSaveGameList_->deleteAllItems();

    // Add previously saved games into list box.
    uint numSavedGames = MachGuiDatabase::instance().nSavedGames();

    for (uint loop = 0; loop < numSavedGames; ++loop)
    {
        MachGuiDbSavedGame& savedGame = MachGuiDatabase::instance().savedGame(loop);
        new MachGuiLoadGameListBoxItem(
            pStartupScreens_,
            pSaveGameList_,
            MachGui::menuScaleFactor() * (LOAD_LB_MAXX - LOAD_LB_MINX - SCROLLBAR_WIDTH),
            savedGame,
            this);
    }

    // Get list to redraw.
    pSaveGameList_->childrenUpdated();
}

// virtual
bool MachGuiCtxLoad::okayToSwitchContext()
{
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::OK)
    {
        // Unload game if one is currently being played.
        pStartupScreens_->unloadGame();

        // Load game
        pStartupScreens_->loadSavedGame(pSelectedSaveGame_);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

class MachGuiLoadMessageBoxResponder : public MachGuiMessageBoxResponder
{
public:
    MachGuiLoadMessageBoxResponder(MachGuiCtxLoad* pLoadCtx)
        : pLoadCtx_(pLoadCtx)
    {
    }

    bool okPressed() override
    {
        // if ( MachGui::machinesCDIsAvailable( 2 ) )
        {
            pLoadCtx_->autoLoadGame();
            return true;
        }
        // return false;
    }

private:
    MachGuiCtxLoad* pLoadCtx_;
};

///////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiCtxLoad::buttonEvent(MachGui::ButtonEvent be)
{
    if (be == MachGui::ButtonEvent::DELETE)
    {
        if (pSelectedSaveGame_)
        {
            // Check that current player created this game and can therefore delete it.
            if (MachGuiDatabase::instance().hasCurrentPlayer() && pSelectedSaveGame_->hasPlayer()
                && &pSelectedSaveGame_->player() != &MachGuiDatabase::instance().currentPlayer())
            {
                GuiStrings strings;
                strings.push_back(MachGuiDatabase::instance().currentPlayer().name());
                pStartupScreens_->displayMsgBox(IDS_MENUMSG_SAVEDGAMENOTPLAYERS, strings);
            }
            else
            {
                pStartupScreens_->displayMsgBox(
                    IDS_MENUMSG_DELETESAVEDGAME,
                    new MachGuiLoadGameDeleteMessageBoxResponder(this));
            }
        }
    }
    else if (be == MachGui::ButtonEvent::DUMMY_OK)
    {
        if (!pSelectedSaveGame_)
        {
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_SELECTSAVEDGAME);
        }
        else
        {
            pStartupScreens_->buttonAction(MachGui::ButtonEvent::OK);
        }
    }
}

void MachGuiCtxLoad::selectedSaveGame(MachGuiDbSavedGame* pSavedGame)
{
    pSelectedSaveGame_ = pSavedGame;
    pDeleteBtn_->disabled(false);
}

void MachGuiCtxLoad::deleteSavedGame()
{
    PRE(pSelectedSaveGame_);

    remove(pSelectedSaveGame_->externalFileName().c_str());
    MachGuiDatabase::instance().removeSavedGame(pSelectedSaveGame_);
    MachGuiDatabase::instance().writeDatabase();
    updateSaveGameList();
}

void MachGuiCtxLoad::autoLoadGame()
{
    autoLoadGame_ = true;
}

/* End CTXLOAD.CPP **************************************************/
