/*
 * P R O M T E X T . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/prepost.hpp"
#include "ctl/list.hpp"
#include "ctl/pvector.hpp"
#include "machgui/gui.hpp"
#include "machgui/promtext.hpp"
#include "machgui/cameras.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/chatmsgs.hpp"
#include "machgui/ingame.hpp"
#include "machlog/network.hpp"
#include "machlog/races.hpp"
#include "machlog/pod.hpp"
#include "machlog/canattac.hpp"
#include "machlog/weapon.hpp"
#include "machlog/nuclbomb.hpp"
#include "machlog/missilem.hpp"
#include "machlog/restree.hpp"
#include "machlog/resitem.hpp"
#include "machphys/objdata.hpp"
#include "machphys/wepdata.hpp"
#include "machphys/mphydata.hpp"
#include "gui/manager.hpp"
#include "gui/event.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "gui/restring.hpp"
#include "device/butevent.hpp"
#include "machgui/internal/strings.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "render/device.hpp"
#include "render/surfmgr.hpp"
#include "device/cd.hpp"
#include <string>
// #include <mbstring.h>

#define SYSTEM_MESSAGE 100

using strings = std::vector<std::string>;

bool checkEnteredText(const unsigned char* pEncryptedText, const unsigned char* pDecryptKey, const string& enteredText)
{
    //  int sizeOfEncryptedText = _mbslen( pEncryptedText );
    int sizeOfEncryptedText = 10; // Magic codes, have a fun decrypting them - martin.

    string decryptedText;

    if (sizeOfEncryptedText != enteredText.length())
    {
        return false;
    }

    for (int i = 0; i < sizeOfEncryptedText; ++i)
    {
        decryptedText += pEncryptedText[i] ^ pDecryptKey[i];
    }

    return enteredText == decryptedText;
}

class MachPromptTextImpl
{
public:
    MachPromptTextImpl(
        const std::string& normalFont,
        const std::string& shadowFont,
        GuiBmpFont::FontType fontType,
        size_t spaceCharWidth,
        size_t spacing);

    GuiBitmap promptBmp_;
    string cursorPromptText_; // The prompt displayed for mouse moves
    string commandPromptText_; // The prompt displayed for current command
    strings cursorPromptTextLines_;
    strings commandPromptTextLines_;
    MachCameras* pCameras_;
    bool displayCursorPromptText_;
    MachPromptText::PromptDisplayed promptDisplayed_;
    GuiBmpFont font_;
    GuiBmpFont shadowFont_;
    int blitToX_;
    bool refresh_;
    bool restartScroll_;
    GuiBitmap lightOn_;
    GuiBitmap lightOff_;
    bool enteringChatMessage_;
    MachPhys::Race chatMessageIntendedForRace_;
    string chatMessageIntendedForStr_;
    string lastDisplayedChatMessage_;
    int beginningTextWidth_;
    GuiDisplayable* pPassEventsTo_;
    int standardMessageIndex_;
    int opponentIndex_;
    MachInGameScreen* pInGameScreen_;
};

constexpr int c_textScrollSpeed = 20;

MachPromptTextImpl::MachPromptTextImpl(
    const std::string& normalFont,
    const std::string& shadowFont,
    GuiBmpFont::FontType fontType,
    size_t spaceCharWidth,
    size_t spacing)
    : font_(GuiBmpFont::getFont(normalFont, fontType, spaceCharWidth, spacing))
    , shadowFont_(GuiBmpFont::getFont(shadowFont, fontType, spaceCharWidth, spacing))
    , refresh_(true)
    , lightOn_(Gui::bitmap(MachGui::getScaledImagePath("gui/misc/tplight2")))
    , lightOff_(Gui::bitmap(MachGui::getScaledImagePath("gui/misc/tplight1")))
    , enteringChatMessage_(false)
    , chatMessageIntendedForRace_(MachPhys::N_RACES)
    , beginningTextWidth_(0)
    , pPassEventsTo_(nullptr)
    , standardMessageIndex_(0)
    , pInGameScreen_(nullptr)
{
    lightOn_.enableColourKeying();
    lightOff_.enableColourKeying();
}

MachPromptText::MachPromptText(
    MachInGameScreen* pParent,
    const Gui::Boundary& relativeBoundary,
    MachCameras* pCameras,
    GuiDisplayable* pPassEventsTo)
    : GuiSingleLineEditBox(pParent, relativeBoundary)
{
    std::string normalFont = MachGui::getScaledImagePath("gui/menu/promtfnt");
    std::string shadowFont = MachGui::getScaledImagePath("gui/menu/promdfnt");
    GuiBmpFont::FontType fontType = GuiBmpFont::PROPORTIONAL;
    size_t spaceCharWidth = 7 * Gui::uiScaleFactor();
    size_t spacing = 1 * Gui::uiScaleFactor();

    font_ = GuiBmpFont::getFont(normalFont, fontType, spaceCharWidth, spacing);
    pImpl_ = new MachPromptTextImpl(normalFont, shadowFont, fontType, spaceCharWidth, spacing);

    CB_DEPIMPL(GuiBitmap, promptBmp_);
    CB_DEPIMPL(MachCameras*, pCameras_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(int, blitToX_);
    CB_DEPIMPL(GuiDisplayable*, pPassEventsTo_);
    CB_DEPIMPL(MachInGameScreen*, pInGameScreen_);

    setLayer(GuiDisplayable::LAYER4);

    promptBmp_ = RenSurface::createAnonymousSurface(width(), height(), Gui::backBuffer());
    promptBmp_.enableColourKeying();
    pCameras_ = pCameras;
    promptDisplayed_ = NOTEXT;
    blitToX_ = 0;
    pPassEventsTo_ = pPassEventsTo;
    pInGameScreen_ = pParent;

    redrawEveryFrame(true);

    maxChars(MAX_INGAMECHATMESSAGE_LEN - MAX_PLAYERNAME_LEN - 5);

    TEST_INVARIANT;
}

MachPromptText::~MachPromptText()
{
    TEST_INVARIANT;
    delete pImpl_;
}

void MachPromptText::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPromptText& t)
{

    o << "MachPromptText " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPromptText " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachPromptText::setCursorPromptText(const string& prompt)
{
    setCursorPromptText(prompt, true);
}

void MachPromptText::setCursorPromptText(const string& prompt, bool restartScroll)
{
    CB_DEPIMPL(string, cursorPromptText_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);
    CB_DEPIMPL(strings, cursorPromptTextLines_);
    CB_DEPIMPL(GuiBmpFont, font_);

    if (prompt != cursorPromptText_)
    {
        cursorPromptText_ = prompt;

        cursorPromptTextLines_.erase(cursorPromptTextLines_.begin(), cursorPromptTextLines_.end());
        MachGuiMenuText::chopUpText(prompt, 2000, font_, &cursorPromptTextLines_);

        refresh_ = true;

        if (promptDisplayed_ != CURSORTEXT)
        {
            restartScroll_ = true;
        }
        else
        {
            restartScroll_ = restartScroll;
        }
    }
}

const string& MachPromptText::cursorPromptText() const
{
    CB_DEPIMPL(string, cursorPromptText_);

    return cursorPromptText_;
}

void MachPromptText::clearCursorPromptText()
{
    CB_DEPIMPL(string, cursorPromptText_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);

    if (cursorPromptText_ != "")
    {
        cursorPromptText_ = "";
        promptDisplayed_ = NOTEXT;
    }
}

void MachPromptText::setCommandPromptText(const string& prompt)
{
    CB_DEPIMPL(string, commandPromptText_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);
    CB_DEPIMPL(strings, commandPromptTextLines_);
    CB_DEPIMPL(GuiBmpFont, font_);

    if (commandPromptText_ != prompt)
    {
        commandPromptText_ = prompt;

        commandPromptTextLines_.erase(commandPromptTextLines_.begin(), commandPromptTextLines_.end());
        MachGuiMenuText::chopUpText(prompt, 2000, font_, &commandPromptTextLines_);

        if (promptDisplayed_ != CURSORTEXT)
        {
            refresh_ = true; // Force prompt text to be refreshed
            restartScroll_ = true;
        }
    }
}

const string& MachPromptText::commandPromptText() const
{
    CB_DEPIMPL(string, commandPromptText_);

    return commandPromptText_;
}

void MachPromptText::clearCommandPromptText()
{
    CB_DEPIMPL(string, commandPromptText_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);

    if (commandPromptText_ != "")
    {
        commandPromptText_ = "";

        if (promptDisplayed_ == COMMANDTEXT)
        {
            promptDisplayed_ = NOTEXT;
        }
    }
}

void MachPromptText::doDisplay()
{
    CB_DEPIMPL(string, commandPromptText_);
    CB_DEPIMPL(string, cursorPromptText_);
    CB_DEPIMPL(strings, cursorPromptTextLines_);
    CB_DEPIMPL(strings, commandPromptTextLines_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(GuiBitmap, promptBmp_);
    CB_DEPIMPL(GuiBitmap, lightOn_);
    CB_DEPIMPL(GuiBitmap, lightOff_);
    CB_DEPIMPL(GuiBmpFont, font_);
    CB_DEPIMPL(GuiBmpFont, shadowFont_);
    CB_DEPIMPL(int, blitToX_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);
    CB_DEPIMPL(bool, enteringChatMessage_);

    if (enteringChatMessage_)
    {
        displayChatMessage();
        return;
    }
    // Decide which text to display. Cursor prompt takes priority
    if (cursorPromptText_.length() != 0)
    {
        displayPromptText(CURSORTEXT, cursorPromptTextLines_);
    }
    else if (commandPromptText_.length() != 0)
    {
        displayPromptText(COMMANDTEXT, commandPromptTextLines_);
    }
    else
    {
        promptDisplayed_ = NOTEXT;

        // Scroll text back into control panel
        if (blitToX_ != 0)
        {
            {
                blitToX_ -= std::min(blitToX_, c_textScrollSpeed);

                // Blit text.
                GuiPainter::instance().blit(
                    promptBmp_,
                    Gui::Box(0, 0, blitToX_, promptBmp_.height()),
                    getPromptTextAbsolutePosition());
            }
        }

        // Blit light off graphic
        GuiPainter::instance().blit(lightOff_, absoluteBoundary().minCorner());
    }
}

// virtual
bool MachPromptText::doHandleCharEvent(const GuiCharEvent& event)
{
    CB_DEPIMPL(bool, enteringChatMessage_);

    if (enteringChatMessage_)
        return GuiSingleLineEditBox::doHandleCharEvent(event);

    return false;
}

void MachPromptText::displayChatMessage()
{
    CB_DEPIMPL(GuiBitmap, promptBmp_);
    CB_DEPIMPL(GuiBitmap, lightOn_);
    CB_DEPIMPL(GuiBmpFont, font_);
    CB_DEPIMPL(GuiBmpFont, shadowFont_);
    CB_DEPIMPL(string, chatMessageIntendedForStr_);
    CB_DEPIMPL(string, lastDisplayedChatMessage_);
    CB_DEPIMPL(int, beginningTextWidth_);

    // Update the caret
    update();

    string displayChatMessageStr = chatMessageIntendedForStr_ + text();
    int caretPosition = caretPos() + beginningTextWidth_;
    int startY = shadowFont_.charHeight() / 2;

    // Make sure we only update when the text has changed
    if (lastDisplayedChatMessage_ != displayChatMessageStr)
    {
        lastDisplayedChatMessage_ = displayChatMessageStr;

        promptBmp_.filledRectangle(RenSurface::Rect(0, 0, promptBmp_.width(), promptBmp_.height()), Gui::MAGENTA());

        // Work out starting position for text
        Gui::Coord startBeginningText = Gui::Coord(0, startY);
        Gui::Coord shadowStartBeginningText = Gui::Coord(1, startY + 1);
        Gui::Coord startText = Gui::Coord(caretPosition, startY);
        Gui::Coord shadowStartText = Gui::Coord(caretPosition + 1, startY + 1);

        // Draw shadow and normal text
        shadowFont_.drawText(&promptBmp_, chatMessageIntendedForStr_, shadowStartBeginningText, promptBmp_.width());
        shadowFont_.drawText(
            &promptBmp_,
            rightText(),
            shadowStartText,
            promptBmp_.width() - caretPosition - 1 - beginningTextWidth_);
        shadowFont_.drawText(
            &promptBmp_,
            leftText(),
            shadowStartText,
            caretPosition - beginningTextWidth_,
            GuiBmpFont::RIGHT_JUSTIFY);
        font_.drawText(&promptBmp_, chatMessageIntendedForStr_, startBeginningText, promptBmp_.width());
        font_.drawText(&promptBmp_, rightText(), startText, promptBmp_.width() - caretPosition - beginningTextWidth_);
        font_.drawText(
            &promptBmp_,
            leftText(),
            startText,
            caretPosition - beginningTextWidth_,
            GuiBmpFont::RIGHT_JUSTIFY);
    }

    // Blit text.
    GuiPainter::instance().blit(
        promptBmp_,
        Gui::Box(0, 0, promptBmp_.width(), promptBmp_.height()),
        getPromptTextAbsolutePosition());

    if (GuiManager::instance().charFocusExists() && &GuiManager::instance().charFocus() == this
        && showCaret()) // Only show caret if we have focus
    {
        Gui::Coord from = getPromptTextAbsolutePosition() + Gui::Vec(caretPosition, startY);
        Gui::Coord to = from + Gui::Vec(0, font_.charHeight());
        GuiPainter::instance().line(from, to, caretColour(), 1 * Gui::uiScaleFactor());
    }

    // Blit light on graphic
    GuiPainter::instance().blit(lightOn_, absoluteBoundary().minCorner());
}

void MachPromptText::displayPromptText(PromptDisplayed textType, const std::vector<std::string>& textLines)
{
    if (pImpl_->refresh_ || pImpl_->promptDisplayed_ != textType)
    {
        pImpl_->promptDisplayed_ = textType;
        pImpl_->refresh_ = false;
        pImpl_->promptBmp_.filledRectangle(
            RenSurface::Rect(0, 0, pImpl_->promptBmp_.width(), pImpl_->promptBmp_.height()),
            Gui::MAGENTA());

        // Render all the lines
        int startY = 0;
        if (textLines.size() == 1)
        {
            startY += pImpl_->shadowFont_.charHeight() / 2;
        }

        for (const std::string& line : textLines)
        {
            Gui::Coord textPos(0, startY);
            Gui::Coord shadowPos = textPos + Gui::Vec(1, 1) * Gui::uiScaleFactor();
            pImpl_->shadowFont_
                .drawText(&pImpl_->promptBmp_, line, shadowPos, pImpl_->promptBmp_.width() - shadowPos.x());
            pImpl_->font_.drawText(&pImpl_->promptBmp_, line, textPos, pImpl_->promptBmp_.width());
            startY += pImpl_->shadowFont_.charHeight() + 1 * Gui::uiScaleFactor();
        }

        if (pImpl_->restartScroll_)
        {
            pImpl_->blitToX_ = 0;
        }
    }

    // Blit text.
    GuiPainter::instance().blit(
        pImpl_->promptBmp_,
        Gui::Box(0, 0, pImpl_->blitToX_, pImpl_->promptBmp_.height()),
        getPromptTextAbsolutePosition());

    // Scroll text to new pos for next frame.
    if (pImpl_->blitToX_ != pImpl_->promptBmp_.width())
    {
        pImpl_->blitToX_ += c_textScrollSpeed;
        if (pImpl_->blitToX_ >= width())
        {
            pImpl_->blitToX_ = pImpl_->promptBmp_.width();
        }

        // Blit light on graphic
        GuiPainter::instance().blit(pImpl_->lightOn_, absoluteBoundary().minCorner());
    }
    else
    {
        // Blit light off graphic
        GuiPainter::instance().blit(pImpl_->lightOff_, absoluteBoundary().minCorner());
    }
}

// virtual
bool MachPromptText::doHandleKeyEvent(const GuiKeyEvent& event)
{
    CB_DEPIMPL(bool, enteringChatMessage_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);
    CB_DEPIMPL(MachPhys::Race, chatMessageIntendedForRace_);
    CB_DEPIMPL(string, chatMessageIntendedForStr_);
    CB_DEPIMPL(int, beginningTextWidth_);
    CB_DEPIMPL(GuiBmpFont, shadowFont_);
    CB_DEPIMPL(int, opponentIndex_);
    CB_DEPIMPL(int, standardMessageIndex_);
    CB_DEPIMPL(MachInGameScreen*, pInGameScreen_);

    bool processed = false;

    if (event.state() == Gui::PRESSED)
    {
        if ((event.key() == Device::KeyCode::F1 || event.key() == Device::KeyCode::F2 || event.key() == Device::KeyCode::F3)
            && ! event.isShiftPressed() && MachLogNetwork::instance().isNetworkGame())
        {
            // Work out who we are going to send the message to
            int newOpponentIndex = 0;

            if (event.key() == Device::KeyCode::F2)
                newOpponentIndex = 1;
            else if (event.key() == Device::KeyCode::F3)
                newOpponentIndex = 2;

            if (event.isCtrlPressed())
            {
                if (MachGuiInGameChatMessages::instance().opponentExists(newOpponentIndex))
                {
                    MachPhys::Race targetRace = MachGuiInGameChatMessages::instance().opponentRace(newOpponentIndex);
                    MachPhys::Race ourRace = MachLogRaces::instance().playerRace();
                    MachLogRaces::DispositionToRace disp
                        = MachLogRaces::instance().dispositionToRace(ourRace, targetRace);
                    MachLogRaces::DispositionToRace newDisp = MachLogRaces::ALLY;
                    if (disp == MachLogRaces::ALLY)
                        newDisp = MachLogRaces::NEUTRAL;
                    MachLogRaces::instance().dispositionToRace(ourRace, targetRace, newDisp);
                }
            }
            else
            {
                // Are we already entering a message and want to cycle through a standard list of messages?
                if (enteringChatMessage_ && opponentIndex_ == newOpponentIndex)
                {
                    if (MachGuiInGameChatMessages::instance().standardMessages().size() > 0)
                    {
                        if (standardMessageIndex_ >= MachGuiInGameChatMessages::instance().standardMessages().size())
                        {
                            standardMessageIndex_ = 0;
                        }

                        setText(MachGuiInGameChatMessages::instance().standardMessages()[standardMessageIndex_]);
                        // Display next message next time
                        ++standardMessageIndex_;
                    }
                }
                else // A new message ( or new opponent )...
                {
                    opponentIndex_ = newOpponentIndex;
                    standardMessageIndex_ = 0;

                    if (MachGuiInGameChatMessages::instance().opponentExists(opponentIndex_))
                    {
                        enteringChatMessage_ = true;
                        GuiManager::instance().charFocus(this);
                        chatMessageIntendedForRace_
                            = MachGuiInGameChatMessages::instance().opponentRace(opponentIndex_);

                        // Add coloured token to beginning of message
                        switch (chatMessageIntendedForRace_)
                        {
                            case MachPhys::RED:
                                chatMessageIntendedForStr_.assign(1, GuiBmpFont::redCharIndex());
                                break;
                            case MachPhys::GREEN:
                                chatMessageIntendedForStr_.assign(1, GuiBmpFont::greenCharIndex());
                                break;
                            case MachPhys::BLUE:
                                chatMessageIntendedForStr_.assign(1, GuiBmpFont::blueCharIndex());
                                break;
                            case MachPhys::YELLOW:
                                chatMessageIntendedForStr_.assign(1, GuiBmpFont::yellowCharIndex());
                                break;
                            default:
                                ASSERT_FAIL("Invalid race for a chat message");
                                break;
                        }
                        GuiResourceString sendToStr(IDS_SENDTO);
                        chatMessageIntendedForStr_ += sendToStr.asString();
                        chatMessageIntendedForStr_
                            += MachGuiInGameChatMessages::instance().opponentName(opponentIndex_);
                        chatMessageIntendedForStr_ += ": ";
                        beginningTextWidth_ = shadowFont_.textWidth(chatMessageIntendedForStr_);
                    }
                }
            }
        }
        else if (
            event.key() == Device::KeyCode::F4 && ! event.isShiftPressed() && MachLogNetwork::instance().isNetworkGame())
        {
            int newOpponentIndex = 3;

            // Are we already entering a message and want to cycle through a standard list of messages?
            if (enteringChatMessage_ && opponentIndex_ == newOpponentIndex)
            {
                if (MachGuiInGameChatMessages::instance().standardMessages().size() > 0)
                {
                    if (standardMessageIndex_ >= MachGuiInGameChatMessages::instance().standardMessages().size())
                    {
                        standardMessageIndex_ = 0;
                    }

                    setText(MachGuiInGameChatMessages::instance().standardMessages()[standardMessageIndex_]);
                    // Display next message next time
                    ++standardMessageIndex_;
                }
            }
            else
            {
                standardMessageIndex_ = 0;
                opponentIndex_ = newOpponentIndex;
                enteringChatMessage_ = true;
                GuiManager::instance().charFocus(this);
                chatMessageIntendedForRace_ = MachPhys::N_RACES;

                GuiResourceString sendToEveryoneStr(IDS_SENDTOEVERYONE);
                chatMessageIntendedForStr_ = sendToEveryoneStr.asString();
                beginningTextWidth_ = shadowFont_.textWidth(chatMessageIntendedForStr_);
            }
        }
        else if (
            event.key() == Device::KeyCode::F11 && event.isShiftPressed() && event.isCtrlPressed() && event.isAltPressed())
        {
            opponentIndex_ = SYSTEM_MESSAGE;
            enteringChatMessage_ = true;
            GuiManager::instance().charFocus(this);
            chatMessageIntendedForRace_ = MachPhys::N_RACES;

            GuiResourceString sendToSystemStr(IDS_SENDTOSYSTEM);
            chatMessageIntendedForStr_ = sendToSystemStr.asString();
            beginningTextWidth_ = shadowFont_.textWidth(chatMessageIntendedForStr_);
        }
        else if (event.key() == Device::KeyCode::ESCAPE || event.key() == Device::KeyCode::ENTER || event.key() == Device::KeyCode::F10)
        {
            if (enteringChatMessage_)
            {
                // Send a chat message
                if (event.key() == Device::KeyCode::ENTER)
                {
                    if (opponentIndex_ != SYSTEM_MESSAGE)
                    {
                        string chatMessageStr;

                        // Add coloured token to beginning of message
                        switch (MachGuiInGameChatMessages::instance().playerRace())
                        {
                            case MachPhys::RED:
                                chatMessageStr += GuiBmpFont::redCharIndex();
                                break;
                            case MachPhys::GREEN:
                                chatMessageStr += GuiBmpFont::greenCharIndex();
                                break;
                            case MachPhys::BLUE:
                                chatMessageStr += GuiBmpFont::blueCharIndex();
                                break;
                            case MachPhys::YELLOW:
                                chatMessageStr += GuiBmpFont::yellowCharIndex();
                                break;
                        }
                        chatMessageStr += MachGuiInGameChatMessages::instance().playerName();
                        chatMessageStr += ": ";
                        chatMessageStr += text();

                        MachGuiInGameChatMessages::instance().sendMessage(chatMessageStr, chatMessageIntendedForRace_);
                        MachGuiInGameChatMessages::instance().addMessage(chatMessageIntendedForStr_ + text());
                    }
                    else
                    {
                        //                      if ( strnicmp( text().c_str(), "MUSIC", 5 ) == 0 and text().length() > 5
                        //                      )
                        if (strncasecmp(text().c_str(), "MUSIC", 5) == 0 && text().length() > 5)
                        {
                            int track = atoi(&text().c_str()[5]);

                            if (track > 0 && // Valid track number entered
                                DevCD::instance().isAudioCDPresent() && // Audio CD is in CD-Rom
                                track < DevCD::instance()
                                            .numberOfTracks()) // Track number is not outside the number of tracks on CD
                            {
                                DevCD::instance().play(track);
                            }
                        }
                        else if (strcasecmp(text().c_str(), "EXIT") == 0) // Instant Exit
                        {
                            pInGameScreen_->instantExit(true);
                        }
                    }
                }

                // Clear message from prompt text
                enteringChatMessage_ = false;
                GuiManager::instance().removeCharFocus();
                setText("");
                refresh_ = true;
                restartScroll_ = true;
                processed = true;
            }
        }
        else if (enteringChatMessage_)
        {
            processed = GuiSingleLineEditBox::doHandleKeyEvent(event);
        }
    }

    return processed || enteringChatMessage_;
}

// virtual
int MachPromptText::maxWidth() const
{
    // Work out how much room there is to type in a chat message
    CB_DEPIMPL(int, beginningTextWidth_);
    CB_DEPIMPL(GuiBitmap, lightOn_);

    // Work out how much of the prompt text is being displayed on-screen
    const int w = W4dManager::instance().sceneManager()->pDevice()->windowWidth();
    Gui::XCoord promptTextXPos = absoluteBoundary().minCorner().x();

    int availableWidth = w - promptTextXPos;

    // Take into account other things that limit the available typing space
    return availableWidth - beginningTextWidth_ - lightOn_.width() - 1;
}

Gui::Coord MachPromptText::getPromptTextAbsolutePosition() const
{
    CB_DEPIMPL(GuiBitmap, lightOn_);

    const int yOffset = 7 * Gui::uiScaleFactor();
    const int w = W4dManager::instance().sceneManager()->pDevice()->windowWidth();
    const int baseSpacing = w < 1024 ? 1 : 2;
    const int hSpacing = baseSpacing * Gui::uiScaleFactor();
    const int xOffset = lightOn_.width() + hSpacing;

    return absoluteBoundary().minCorner() + Gui::Vec(xOffset, yOffset);
}

/* NA 30/11/98. New processesMouseEvents added to GuiDisplayable means I don't need these
//virtual
void MachPromptText::doHandleMouseClickEvent( const GuiMouseEvent& rel )
{
    CB_DEPIMPL( GuiDisplayable*, pPassEventsTo_ );

    GuiMouseEvent relToOther = rel;

    if ( passEventTo( &relToOther ) )
    {
        pPassEventsTo_->doHandleMouseClickEvent( relToOther );
    }
}

//virtual
void MachPromptText::doHandleMouseEnterEvent( const GuiMouseEvent& rel )
{
    CB_DEPIMPL( GuiDisplayable*, pPassEventsTo_ );

    GuiMouseEvent relToOther = rel;

    if ( passEventTo( &relToOther ) )
    {
        pPassEventsTo_->doHandleMouseEnterEvent( relToOther );
    }
}

//virtual
void MachPromptText::doHandleMouseExitEvent( const GuiMouseEvent& rel )
{
    CB_DEPIMPL( GuiDisplayable*, pPassEventsTo_ );

    GuiMouseEvent relToOther = rel;

    if ( passEventTo( &relToOther ) )
    {
        pPassEventsTo_->doHandleContainsMouseEvent( relToOther );
    }
    else
    {
        pPassEventsTo_->doHandleMouseExitEvent( relToOther );
    }
}

//virtual
void MachPromptText::doHandleContainsMouseEvent( const GuiMouseEvent& rel )
{
    CB_DEPIMPL( GuiDisplayable*, pPassEventsTo_ );

    GuiMouseEvent relToOther = rel;

    if ( passEventTo( &relToOther ) )
    {
        pPassEventsTo_->doHandleContainsMouseEvent( relToOther );
    }
}

bool MachPromptText::passEventTo( GuiMouseEvent* pMouseEvent )
{
    CB_DEPIMPL( GuiDisplayable*, pPassEventsTo_ );

    bool returnValue = false;

    Gui::Coord absEventPos = absoluteBoundary().minCorner();
    absEventPos.x( absEventPos.x() + pMouseEvent->coord().x() );
    absEventPos.y( absEventPos.y() + pMouseEvent->coord().y() );

    if ( pPassEventsTo_->contains( absEventPos ) )
    {
        pMouseEvent->translate( Gui::Vec( ( absEventPos.x() - pPassEventsTo_->absoluteCoord().x() ) -
pMouseEvent->coord().x(), ( absEventPos.y() - pPassEventsTo_->absoluteCoord().y() ) - pMouseEvent->coord().y() ) );
        returnValue = true;
    }

    return returnValue;
}
*/

// virtual
bool MachPromptText::processesMouseEvents() const
{
    return false;
}

// Forced recompile 16/2/99 CPS
/* End PROMTEXT.CPP *************************************************/
