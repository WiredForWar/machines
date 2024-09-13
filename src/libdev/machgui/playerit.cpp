/*
 * P L A Y E R I T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/playerit.hpp"

#include "base/diag.hpp"
#include "machgui/startup.hpp"
#include "machgui/gui.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/messbrok.hpp"
#include "machgui/dropdown.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "gui/scrolist.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "gui/event.hpp"
#include "machgui/menus_helper.hpp"

// virtual
void MachGuiColourList::doDisplay()
{
}

// virtual
bool MachGuiColourList::containsMousePointer()
{
    Gui::Coord mousePos(DevMouse::instance().position().first, DevMouse::instance().position().second);

    return contains(mousePos);
}

MachGuiColourSelector::MachGuiColourSelector(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiColourList* pParent,
    const Gui::Box& box,
    MachPhys::Race race,
    MachGuiSharedPlayerInfo* pPlayerInfo)
    : GuiDisplayable(pParent, box)
    , highlighted_(false)
    , pStartupScreens_(pStartupScreens)
    , race_(race)
    , pColourList_(pParent)
    , pPlayerInfo_(pPlayerInfo)
{
    // Check to see if the colour is available ( i.e. hasn't been taken by another player ).
    if (! pStartupScreens->startupData()->isRaceAvailable(race) && race != pPlayerInfo->race_)
    {
        race_ = MachPhys::NORACE;
    }
}

MachGuiColourSelector::~MachGuiColourSelector()
{
}

// static
void MachGuiPlayerColour::releaseBmpMemory()
{
    // Free up all the memory used by the cached bitmaps
    GuiBitmap*& pBmp1 = pRedBmp();
    GuiBitmap*& pBmp2 = pBlueBmp();
    GuiBitmap*& pBmp3 = pGreenBmp();
    GuiBitmap*& pBmp4 = pYellowBmp();
    GuiBitmap*& pBmp5 = pBlackBmp();

    delete pBmp1;
    delete pBmp2;
    delete pBmp3;
    delete pBmp4;
    delete pBmp5;

    pBmp1 = nullptr;
    pBmp2 = nullptr;
    pBmp3 = nullptr;
    pBmp4 = nullptr;
    pBmp5 = nullptr;
}

// static
GuiBitmap*& MachGuiPlayerColour::pRedBmp()
{
    static GuiBitmap* pBmp = nullptr;
    if (! pBmp)
    {
        pBmp = new GuiBitmap(MachGui::getScaledImage("gui/menu/gred.bmp", MachGui::menuScaleFactor()));
    }
    return pBmp;
}

// static
GuiBitmap*& MachGuiPlayerColour::pGreenBmp()
{
    static GuiBitmap* pBmp = nullptr;
    if (! pBmp)
    {
        pBmp = new GuiBitmap(MachGui::getScaledImage("gui/menu/ggreen.bmp", MachGui::menuScaleFactor()));
    }
    return pBmp;
}

// static
GuiBitmap*& MachGuiPlayerColour::pBlueBmp()
{
    static GuiBitmap* pBmp = nullptr;
    if (! pBmp)
    {
        pBmp = new GuiBitmap(MachGui::getScaledImage("gui/menu/gblue.bmp", MachGui::menuScaleFactor()));
    }
    return pBmp;
}

// static
GuiBitmap*& MachGuiPlayerColour::pYellowBmp()
{
    static GuiBitmap* pBmp = nullptr;
    if (! pBmp)
    {
        pBmp = new GuiBitmap(MachGui::getScaledImage("gui/menu/gyellow.bmp", MachGui::menuScaleFactor()));
    }
    return pBmp;
}

// static
GuiBitmap*& MachGuiPlayerColour::pBlackBmp()
{
    static GuiBitmap* pBmp = nullptr;
    if (! pBmp)
    {
        pBmp = new GuiBitmap(MachGui::getScaledImage("gui/menu/gblack.bmp", MachGui::menuScaleFactor()));
    }
    return pBmp;
}

// virtual
void MachGuiColourSelector::doDisplay()
{
    const GuiColour* pGlowColour = nullptr;
    const GuiColour* pDarkColour = nullptr;

    switch (race_)
    {
        case MachPhys::RED:
            pGlowColour = &Gui::RED();
            pDarkColour = &MachGui::DARKRED();
            break;
        case MachPhys::GREEN:
            pGlowColour = &Gui::GREEN();
            pDarkColour = &MachGui::DARKGREEN();
            break;
        case MachPhys::BLUE:
            pGlowColour = &Gui::BLUE();
            pDarkColour = &MachGui::DARKBLUE();
            break;
        case MachPhys::YELLOW:
            pGlowColour = &Gui::YELLOW();
            pDarkColour = &MachGui::DARKYELLOW();
            break;
        default:
            pGlowColour = &Gui::DARKGREY();
            pDarkColour = &MachGui::ALMOSTBLACK();
            break;
    }

    // Draw background to colour indicator ( glow or normal backdrop ).
    if (highlighted_)
    {
        GuiPainter::instance().filledRectangle(absoluteBoundary(), *pGlowColour);
    }
    else
    {
        GuiPainter::instance().filledRectangle(absoluteBoundary(), *pDarkColour);
    }

    GuiPainter::instance().hollowRectangle(absoluteBoundary(), Gui::BLACK(), 1);
}

// virtual
void MachGuiColourSelector::doHandleMouseEnterEvent(const GuiMouseEvent& /*rel*/)
{
    highlighted_ = true;
    changed();
}

// virtual
void MachGuiColourSelector::doHandleMouseExitEvent(const GuiMouseEvent& /*rel*/)
{
    highlighted_ = false;
    changed();
}

// virtual
void MachGuiColourSelector::doHandleMouseClickEvent(const GuiMouseEvent& rel)
{
    if (rel.leftButton() == Gui::RELEASED)
    {
        if (pStartupScreens_->startupData()->isHost())
        {
            // Host doesn't need to request a race change, it just does
            pStartupScreens_->startupData()->receivedRaceChangeRequest(
                pPlayerInfo_->name_,
                pPlayerInfo_->index_,
                race_);
        }
        else if (! pStartupScreens_->startupData()->terminalMultiPlayerGameProblem())
        {
            // Clients need to ask for a race change, if the host agrees then it will update the race
            pStartupScreens_->messageBroker().sendRaceChangeRequest(pPlayerInfo_->name_, pPlayerInfo_->index_, race_);
        }

        // Next line will cause this to be deleted also.
        delete pColourList_;
    }
}

MachGuiColourList::MachGuiColourList(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiPlayerColour* pParent,
    const Gui::Box& box,
    MachGuiSharedPlayerInfo* pPlayerInfo)
    : MachGuiAutoDeleteDisplayable(pStartupScreens)
    , GuiDisplayable(pParent, box)
{
    constexpr MachPhys::Race colorOptions[]
        = { MachPhys::NORACE, MachPhys::RED, MachPhys::GREEN, MachPhys::BLUE, MachPhys::YELLOW };

    int xPos = 0;
    for (const auto raceOption : colorOptions)
    {
        new MachGuiColourSelector(
            pStartupScreens,
            this,
            Gui::Box(Gui::Coord(xPos, 0) * MachGui::menuScaleFactor(), Gui::Size(13, 13) * MachGui::menuScaleFactor()),
            raceOption,
            pPlayerInfo);
        xPos += 12;
    }
}

MachGuiColourList::~MachGuiColourList()
{
}

MachGuiPlayerListItem::MachGuiPlayerListItem(
    MachGuiStartupScreens* pStartupScreens,
    GuiSimpleScrollableList* pParentList,
    size_t width,
    const std::string& playerName,
    MachPhys::Race playerRace,
    size_t playerIndex,
    bool playerReady,
    bool playerIsHost,
    double ping,
    bool playerHasMachinesCD)
    : GuiDisplayable(
        pParentList,
        Gui::Box(0, 0, static_cast<MATHEX_SCALAR>(width), static_cast<MATHEX_SCALAR>(reqHeight())))
    , highlighted_(false)
    , colourHighlighted_(false)
    , pDropDownList_(nullptr)
    , pStartupScreens_(pStartupScreens)
{
    PRE(playerIndex <= 3);

    pRootParent_ = static_cast<GuiRoot*>(pParentList->findRoot(this));

    // Setup shared player info structure...
    playerInfo_.name_ = playerName;
    playerInfo_.index_ = playerIndex;
    playerInfo_.race_ = playerRace;
    playerInfo_.ready_ = playerReady;
    playerInfo_.host_ = playerIsHost;
    playerInfo_.ping_ = ping;
    playerInfo_.hasMachinesCD_ = playerHasMachinesCD;

    pPlayerColour_ = new MachGuiPlayerColour(pStartupScreens, this, &playerInfo_);
}

MachGuiPlayerListItem::~MachGuiPlayerListItem()
{
}

// static
size_t MachGuiPlayerListItem::reqHeight()
{
    return 16 * MachGui::menuScaleFactor();
}

// static
GuiBmpFont MachGuiPlayerListItem::getFont()
{
    GuiBmpFont bmpFont = GuiBmpFont::getFont(MachGui::Menu::smallFontLight());

    return bmpFont;
}

// static
GuiBmpFont MachGuiPlayerListItem::getHighlightFont()
{
    GuiBmpFont bmpFont = GuiBmpFont::getFont(MachGui::Menu::smallFontDark());

    return bmpFont;
}

void MachGuiPlayerListItem::dropDownListDisplayed(MachGuiDropDownListBox* pList)
{
    pDropDownList_ = pList;
}

// virtual
void MachGuiPlayerListItem::doDisplay()
{
    Gui::YCoord textYOffset = (height() - getFont().charHeight()) / 2.0;

    if (highlighted_ || colourHighlighted_ || pDropDownList_)
    {
        // Draw glowing background
        GuiPainter::instance().blit(
            MachGui::longGlowBmp(),
            Gui::Box(0, 0, width(), height() - 1 * MachGui::menuScaleFactor()),
            absoluteBoundary().minCorner());

        // Blit host icon
        if (playerInfo_.host_)
        {
            GuiPainter::instance().blit(
                MachGui::hostBmp(),
                Gui::Coord(
                    absoluteBoundary().minCorner().x() + 1 * MachGui::menuScaleFactor(),
                    absoluteBoundary().minCorner().y() + 3 * MachGui::menuScaleFactor()));
        }
        else
        {
            // Blit CD icon if machines cd is present in players machine
            if (playerInfo_.hasMachinesCD_)
            {
                GuiBitmap cdBmp = MachGui::getScaledImage("gui/menu/cd.bmp");
                cdBmp.enableColourKeying();

                GuiPainter::instance().blit(
                    cdBmp,
                    Gui::Coord(
                        absoluteBoundary().minCorner().x() + 1 * MachGui::menuScaleFactor(),
                        absoluteBoundary().minCorner().y() + 2 * MachGui::menuScaleFactor()));
            }

            if (playerInfo_.ready_) // Blit tick if necessary.
            {
                GuiPainter::instance().blit(
                    MachGui::darkTickBmp(),
                    Gui::Coord(
                        absoluteBoundary().minCorner().x() + 1 * MachGui::menuScaleFactor(),
                        absoluteBoundary().minCorner().y() + 2 * MachGui::menuScaleFactor()));
            }
        }

        // Draw list box item text
        getHighlightFont().drawText(
            playerInfo_.name_,
            Gui::Coord(
                absoluteBoundary().minCorner().x() + MachGui::darkTickBmp().width() + 2 * MachGui::menuScaleFactor(),
                absoluteBoundary().minCorner().y() + textYOffset),
            width());

        // Only the host can control the names displayed in each slot ( i.e. change from human to computer, open or
        // closed ).
        if (canDisplayDropDownList() && ! pDropDownList_)
        {
            GuiPainter::instance().blit(
                MachGui::dropDownBmp(),
                Gui::Coord(
                    absoluteBoundary().maxCorner().x() - MachGuiPlayerColour::reqWidth()
                        - MachGui::dropDownBmp().width() - 2 * MachGui::menuScaleFactor(),
                    absoluteBoundary().minCorner().y() + 1 * MachGui::menuScaleFactor()));
        }
    }
    else
    {
        // Blit host icon
        if (playerInfo_.host_)
        {
            GuiPainter::instance().blit(
                MachGui::hostBmp(),
                Gui::Coord(
                    absoluteBoundary().minCorner().x() + 1 * MachGui::menuScaleFactor(),
                    absoluteBoundary().minCorner().y() + 3 * MachGui::menuScaleFactor()));
        }
        else
        {
            // Blit CD icon if machines cd is present in players machine
            if (playerInfo_.hasMachinesCD_)
            {
                GuiBitmap cdBmp = MachGui::getScaledImage("gui/menu/cd.bmp");
                cdBmp.enableColourKeying();

                GuiPainter::instance().blit(
                    cdBmp,
                    Gui::Coord(
                        absoluteBoundary().minCorner().x() + 1 * MachGui::menuScaleFactor(),
                        absoluteBoundary().minCorner().y() + 2 * MachGui::menuScaleFactor()));
            }

            if (playerInfo_.ready_) // Blit tick if necessary.
            {
                GuiPainter::instance().blit(
                    MachGui::tickBmp(),
                    Gui::Coord(
                        absoluteBoundary().minCorner().x() + 1 * MachGui::menuScaleFactor(),
                        absoluteBoundary().minCorner().y() + 2 * MachGui::menuScaleFactor()));
            }
        }

        // Draw list box item text
        getFont().drawText(
            playerInfo_.name_,
            Gui::Coord(
                absoluteBoundary().minCorner().x() + MachGui::tickBmp().width() + 2 * MachGui::menuScaleFactor(),
                absoluteBoundary().minCorner().y() + textYOffset),
            width());
    }

    // Show how good/bad ping time is...
    if (playerInfo_.ping_ > 0 && ! pDropDownList_)
    {
        // We only care about ms so multiply by 1000.
        int ping = (int)(playerInfo_.ping_ * 1000);

        GuiBitmap pingBmp;
        if (ping > 800)
            pingBmp = MachGui::getScaledImage("gui/menu/ping1.bmp");
        else if (ping > 600)
            pingBmp = MachGui::getScaledImage("gui/menu/ping2.bmp");
        else if (ping > 250)
            pingBmp = MachGui::getScaledImage("gui/menu/ping3.bmp");
        else if (ping > 100)
            pingBmp = MachGui::getScaledImage("gui/menu/ping4.bmp");
        else
            pingBmp = MachGui::getScaledImage("gui/menu/ping5.bmp");

        pingBmp.enableColourKeying();
        GuiPainter::instance().blit(
            pingBmp,
            Gui::Coord(
                absoluteBoundary().maxCorner().x() - MachGuiPlayerColour::reqWidth() - MachGui::dropDownBmp().width()
                    - pingBmp.width() - 3,
                absoluteBoundary().minCorner().y() + 1));

        // char buffer[10];
        // string pingStr( _itoa( ping, buffer, 10 ) );
        // pingStr += "ms";
        // getFont().drawText( pingStr , Gui::Coord( absoluteBoundary().maxCorner().x() -
        // MachGuiPlayerColour::reqWidth() + 2 - 50, absoluteBoundary().minCorner().y() + textYOffset ), width() );
    }
}

// virtual
void MachGuiPlayerListItem::doHandleMouseEnterEvent(const GuiMouseEvent& /*rel*/)
{
    if (canInteract())
    {
        highlighted_ = true;

        changed();
    }
}

void MachGuiPlayerListItem::colourHighlighted(bool colourHighlight)
{
    colourHighlighted_ = colourHighlight;
    changed();
}

// virtual
void MachGuiPlayerListItem::doHandleMouseExitEvent(const GuiMouseEvent& /*rel*/)
{
    if (canInteract())
    {
        highlighted_ = false;

        changed();
    }
}

bool MachGuiPlayerListItem::canInteract() const
{
    if (pStartupScreens_->startupData()->isHost()
        || (pStartupScreens_->startupData()->playerName() == playerInfo_.name_ && ! playerInfo_.ready_))
    {
        return true;
    }
    return false;
}

void MachGuiPlayerListItem::updateInfo(
    const std::string& playerName,
    MachPhys::Race playerRace,
    bool playerReady,
    bool playerIsHost,
    double ping,
    bool playerHasMachinesCD)
{
    PRE(playerRace == MachPhys::RED || playerRace == MachPhys::BLUE || playerRace == MachPhys::GREEN
        || playerRace == MachPhys::YELLOW || playerRace == MachPhys::NORACE);

    playerInfo_.name_ = playerName;
    playerInfo_.race_ = playerRace;
    playerInfo_.ready_ = playerReady;
    playerInfo_.host_ = playerIsHost;
    playerInfo_.ping_ = ping;
    playerInfo_.hasMachinesCD_ = playerHasMachinesCD;

    if (! pStartupScreens_->msgBoxIsBeingDisplayed())
    {
        changed();
        if (pDropDownList_)
            pDropDownList_->changed();
    }
}

// virtual
void MachGuiPlayerListItem::doHandleContainsMouseEvent(const GuiMouseEvent& /*rel*/)
{
    bool shouldHighlight = false;

    if (canInteract())
    {
        shouldHighlight = true;
    }

    if (shouldHighlight != highlighted_)
    {
        highlighted_ = shouldHighlight;
        changed();
    }
}

void MachGuiPlayerListItem::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiPlayerListItem& t)
{

    o << "MachGuiPlayerListItem " << reinterpret_cast<void*>(const_cast<MachGuiPlayerListItem*>(&t)) << " start"
      << std::endl;
    o << "MachGuiPlayerListItem " << reinterpret_cast<void*>(const_cast<MachGuiPlayerListItem*>(&t)) << " end"
      << std::endl;

    return o;
}

MachGuiPlayerColour::MachGuiPlayerColour(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiPlayerListItem* pParent,
    MachGuiSharedPlayerInfo* pPlayerInfo)
    : GuiDisplayable(pParent, getRelPos(pParent))
    , highlighted_(false)
    , pStartupScreens_(pStartupScreens)
    , pParent_(pParent)
    , pPlayerInfo_(pPlayerInfo)
{
}

MachGuiPlayerColour::~MachGuiPlayerColour()
{
}

// static
size_t MachGuiPlayerColour::reqWidth()
{
    return 61 * MachGui::menuScaleFactor();
}

// static
Gui::Box MachGuiPlayerColour::getRelPos(MachGuiPlayerListItem* pParent)
{
    auto x = static_cast<Gui::XCoord>(pParent->width() - reqWidth() - 1 * MachGui::menuScaleFactor());
    Gui::XCoord y = 1 * MachGui::menuScaleFactor();

    return Gui::Box(x, y, x + reqWidth(), y + pParent->height() - 3 * MachGui::menuScaleFactor());
}

// virtual
void MachGuiPlayerColour::doDisplay()
{
    static uint glowWidth = pRedBmp()->width();
    static uint glowHeight = pRedBmp()->height();

    ASSERT(glowWidth >= width(), "glow bitmap not wide enough");
    ASSERT(glowHeight >= height(), "glow bitmap not high enough");

    uint glowX = (glowWidth - width()) / 2.0;
    uint glowY = (glowHeight - height()) / 2.0;

    const GuiBitmap* pGlowBmp = nullptr;
    const GuiColour* pColour = nullptr;

    switch (pPlayerInfo_->race_)
    {
        case MachPhys::RED:
            pGlowBmp = pRedBmp();
            pColour = &MachGui::DARKRED();
            break;
        case MachPhys::GREEN:
            pGlowBmp = pGreenBmp();
            pColour = &MachGui::DARKGREEN();
            break;
        case MachPhys::BLUE:
            pGlowBmp = pBlueBmp();
            pColour = &MachGui::DARKBLUE();
            break;
        case MachPhys::YELLOW:
            pGlowBmp = pYellowBmp();
            pColour = &MachGui::DARKYELLOW();
            break;
        default:
            pGlowBmp = pBlackBmp();
            pColour = &MachGui::ALMOSTBLACK();
            break;
    }

    // Draw background to colour indicator ( glow or normal backdrop ).
    if (highlighted_)
    {
        GuiPainter::instance().blit(
            *pGlowBmp,
            Gui::Box(Gui::Coord(glowX, glowY), width(), height()),
            absoluteBoundary().minCorner());
    }
    else
    {
        GuiPainter::instance().filledRectangle(absoluteBoundary(), *pColour);
    }

    GuiPainter::instance().hollowRectangle(absoluteBoundary(), Gui::BLACK(), 1 * MachGui::menuScaleFactor());
}

// virtual
void MachGuiPlayerColour::doHandleMouseEnterEvent(const GuiMouseEvent& /*rel*/)
{
    if (pParent_->canInteract() && canInteract())
    {
        highlighted_ = true;
        pParent_->colourHighlighted(true);
        changed();
    }
}

// virtual
void MachGuiPlayerColour::doHandleMouseExitEvent(const GuiMouseEvent& /*rel*/)
{
    if (pParent_->canInteract() && canInteract())
    {
        highlighted_ = false;
        pParent_->colourHighlighted(false);
        changed();
    }
}

// virtual
void MachGuiPlayerColour::doHandleMouseClickEvent(const GuiMouseEvent& rel)
{
    if (pParent_->canInteract() && rel.leftButton() == Gui::RELEASED && canInteract())
    {
        // Only allow colour selection for human and computer
        new MachGuiColourList(pStartupScreens_, this, Gui::Box(0, 0, width(), height()), pPlayerInfo_);
    }
}

// virtual
void MachGuiPlayerColour::doHandleContainsMouseEvent(const GuiMouseEvent& /*rel*/)
{
    bool shouldHighlight = false;

    if (pParent_->canInteract() && canInteract())
    {
        shouldHighlight = true;
    }

    if (shouldHighlight != highlighted_)
    {
        highlighted_ = shouldHighlight;
        pParent_->colourHighlighted(highlighted_);
        changed();
    }
}

bool MachGuiPlayerColour::canInteract() const
{
    if (strcasecmp(pPlayerInfo_->name_.c_str(), MachGui::openText().c_str()) == 0
        || strcasecmp(pPlayerInfo_->name_.c_str(), MachGui::closedText().c_str()) == 0
        || strcasecmp(pPlayerInfo_->name_.c_str(), MachGui::unavailableText().c_str()) == 0)
    {
        return false;
    }

    return true;
}

void MachGuiPlayerColour::refresh()
{
    changed();
}

class MachGuiPlayerSelectionListBox : public MachGuiDropDownListBox
{
public:
    MachGuiPlayerSelectionListBox(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc,
        size_t itemWidth,
        const ctl_vector<std::string>& itemText,
        MachGuiPlayerListItem* pCreator,
        size_t playerIndex)
        : MachGuiDropDownListBox(pParent, box, horizontalSpacing, verticalSpacing, scrollInc, itemWidth, itemText)
        , pCreator_(pCreator)
        , playerIndex_(playerIndex)
    {
        setLayer(GuiDisplayable::LAYER5);
    }

    ~MachGuiPlayerSelectionListBox() override { pCreator_->dropDownListDisplayed(nullptr); }

    void itemSelected(const std::string& text) override
    {
        DEBUG_STREAM(DIAG_NEIL, "MachGuiPlayerSelectionListBox::itemSelected " << text << std::endl);

        MachGuiStartupData::PlayerInfo* pPlayers = startupScreens().startupData()->players();

        // Remove text from player slots ( as long as it ain't "open", "closed", "unavailable" or "computer" )
        bool isPlayersName = strcasecmp(text.c_str(), MachGui::openText().c_str()) != 0
            && strcasecmp(text.c_str(), MachGui::closedText().c_str()) != 0
            && strcasecmp(text.c_str(), MachGui::computerText().c_str()) != 0
            && strcasecmp(text.c_str(), MachGui::unavailableText().c_str()) != 0;

        MachPhys::Race playersRace = MachPhys::NORACE;

        if (isPlayersName)
        {
            for (int loop = 0; loop < 4; ++loop)
            {
                if (strcasecmp(pPlayers[loop].playerName_, text.c_str()) == 0)
                {
                    // Store race that player selected, this will be reassigned when the player is moved to
                    // a new slot
                    playersRace = pPlayers[loop].race_;

                    // Copy player info from slot we are about to wipe over
                    pPlayers[loop].setPlayerName(pPlayers[playerIndex_].getDisplayName());
                    pPlayers[loop].race_ = pPlayers[playerIndex_].race_;
                    break;
                }
            }
        }

        bool isNameAvailable = true;

        // Find out if playersName is still available. Player may have left the multiplayer CtxReady screen
        // whilst the drop-down list has been on screen.
        if (isPlayersName)
        {
            auto iter = find(
                startupScreens().startupData()->availablePlayers().begin(),
                startupScreens().startupData()->availablePlayers().end(),
                text);
            if (iter == startupScreens().startupData()->availablePlayers().end())
            {
                isNameAvailable = false;
            }
        }

        if (isNameAvailable)
        {
            // Add text into player slot
            pPlayers[playerIndex_].setPlayerName(text);

            // Reassign players chosen race
            if (startupScreens().startupData()->isRaceAvailable(playersRace))
            {
                pPlayers[playerIndex_].race_ = playersRace;
            }

            startupScreens().startupData()->sendUpdatePlayersMessage();

            MachGuiDropDownListBox::itemSelected(text);
        }
        else
        {
            // Add open text into player slot
            pPlayers[playerIndex_].setPlayerName(MachGui::openText());

            startupScreens().startupData()->sendUpdatePlayersMessage();

            MachGuiDropDownListBox::itemSelected(MachGui::openText());
        }
    }

private:
    MachGuiPlayerListItem* pCreator_;
    size_t playerIndex_;
};

bool MachGuiPlayerListItem::canDisplayDropDownList() const
{
    MachGuiStartupData* pData = pStartupScreens_->startupData();

    // Must be host...
    if (! pData->isHost())
        return false;

    // Even host cannot remove his name from the player list
    if (strcasecmp(playerInfo_.name_.c_str(), pData->playerName().c_str()) == 0)
        return false;

    // Host cannot interact with "unavailable" slots ( multiplayer scenario may only have
    // 3 slots available, the last slot would be marked as "unavailable" ).
    if (strcasecmp(playerInfo_.name_.c_str(), MachGui::unavailableText().c_str()) == 0)
        return false;

    return true;
}

// virtual
void MachGuiPlayerListItem::doHandleMouseClickEvent(const GuiMouseEvent& rel)
{
    if (rel.leftButton() == Gui::RELEASED && canDisplayDropDownList())
    {
        auto backdrop = pRootParent_->getSharedBitmaps()->getNamedBitmap("backdrop");
        using namespace machgui::helper::menus;
        int menuLeft = x_from_screen_left(pRootParent_->getSharedBitmaps()->getWidthOfNamedBitmap(backdrop), 2);
        int menuTop = y_from_screen_bottom(pRootParent_->getSharedBitmaps()->getHeightOfNamedBitmap(backdrop), 2);

        Gui::Coord dropDownPos = absoluteBoundary().minCorner();
        dropDownPos.x(dropDownPos.x() + MachGui::tickBmp().width() - menuLeft);
        dropDownPos.y(dropDownPos.y() + 4 * MachGui::menuScaleFactor() - menuTop);
        size_t dropDownWidth
            = width() - 3 * MachGui::menuScaleFactor() - MachGui::tickBmp().width() - MachGuiPlayerColour::reqWidth();

        ctl_vector<std::string> strings;

        strings.push_back(playerInfo_.name_); // What ever text is in this item should come first in the drop down

        // Add "open", "closed" and "computer" to the drop down list
        if (strcasecmp(playerInfo_.name_.c_str(), MachGui::openText().c_str()) != 0)
        {
            strings.push_back(MachGui::openText());
        }
        if (strcasecmp(playerInfo_.name_.c_str(), MachGui::closedText().c_str()) != 0)
        {
            strings.push_back(MachGui::closedText());
        }
        if (strcasecmp(playerInfo_.name_.c_str(), MachGui::computerText().c_str()) != 0)
        {
            strings.push_back(MachGui::computerText().c_str());
        }

        // Add all available players names into drop down list
        ctl_vector<std::string>& availPlayers = pStartupScreens_->startupData()->availablePlayers();

        for (auto iter = availPlayers.begin(); iter != availPlayers.end(); ++iter)
        {
            if (strcasecmp((*iter).c_str(), playerInfo_.name_.c_str()) != 0)
            {
                strings.push_back(*iter);
            }
        }

        size_t dropDownHeight = strings.size() * MachGuiSingleSelectionListBoxItem::reqHeight();

        MachGuiDropDownListBox* pDropDownList = new MachGuiPlayerSelectionListBox(
            pStartupScreens_,
            Gui::Box(
                dropDownPos.x(),
                dropDownPos.y(),
                dropDownPos.x() + dropDownWidth,
                dropDownPos.y() + dropDownHeight),
            1000,
            MachGuiSingleSelectionListBoxItem::reqHeight() - 1 /* slight overlap*/,
            1,
            dropDownWidth,
            strings,
            this,
            playerInfo_.index_);

        dropDownListDisplayed(pDropDownList);
    }
}

/* End PLAYERIT.CPP *************************************************/
