/*
 * P R O M T E X T . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "base/PrePost.hpp"
#include "machgui/gui.hpp"
#include "machgui/IInputRegistry.hpp"
#include "machgui/PromptText.hpp"
#include "machgui/Cameras.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/StartupData.hpp"
#include "machgui/InGameChatMessages.hpp"
#include "machgui/InGameScreen.hpp"
#include "machlog/Messaging/Network.hpp"
#include "machlog/Races.hpp"
#include "gui/Manager.hpp"
#include "gui/Event.hpp"
#include "gui/Font.hpp"
#include "gui/GuiPainter.hpp"
#include "gui/ResourceString.hpp"
#include "machgui/internal/strings.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "render/Device.hpp"
#include "render/Painter.hpp"
#include "render/SurfaceManager.hpp"
#include "device/CD.hpp"

#include <string>

#define SYSTEM_MESSAGE 100

using strings = std::vector<std::string>;

class MachPromptTextImpl
{
public:
    MachPromptTextImpl(
        const std::string& normalFont,
        const std::string& shadowFont);

    GuiBitmap promptBmp_;
    std::string cursorPromptText_; // The prompt displayed for mouse moves
    std::string commandPromptText_; // The prompt displayed for current command
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
    std::string chatMessageIntendedForStr_;
    std::string lastDisplayedChatMessage_;
    int beginningTextWidth_;
    GuiDisplayable* pPassEventsTo_;
    int standardMessageIndex_;
    int opponentIndex_;
    MachInGameScreen* pInGameScreen_;
};

constexpr int c_textScrollSpeed = 20;

MachPromptTextImpl::MachPromptTextImpl(
    const std::string& normalFont,
    const std::string& shadowFont)
    : font_(Gui::getFont(normalFont))
    , shadowFont_(Gui::getFont(shadowFont))
    , refresh_(true)
    , lightOn_(MachGui::getScaledImage("gui/misc/tplight2"))
    , lightOff_(MachGui::getScaledImage("gui/misc/tplight1"))
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

    font_ = Gui::getFont(normalFont);
    pImpl_ = new MachPromptTextImpl(normalFont, shadowFont);

    CB_DEPIMPL(GuiBitmap, promptBmp_);
    CB_DEPIMPL(MachCameras*, pCameras_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(int, blitToX_);
    CB_DEPIMPL(GuiDisplayable*, pPassEventsTo_);
    CB_DEPIMPL(MachInGameScreen*, pInGameScreen_);

    setLayer(GuiDisplayable::LAYER4);

    promptBmp_ = RenSurface::createAnonymousSurface(Ren::Size(width(), height()));
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

void MachPromptText::setCursorPromptText(const std::string& prompt)
{
    setCursorPromptText(prompt, true);
}

void MachPromptText::setCursorPromptText(const std::string& prompt, bool restartScroll)
{
    CB_DEPIMPL(std::string, cursorPromptText_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);
    CB_DEPIMPL(strings, cursorPromptTextLines_);
    CB_DEPIMPL(GuiBmpFont, font_);

    if (prompt != cursorPromptText_)
    {
        cursorPromptText_ = prompt;

        cursorPromptTextLines_ = MachGuiMenuText::chopUpText(prompt, 2000, font_);

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

const std::string& MachPromptText::cursorPromptText() const
{
    CB_DEPIMPL(std::string, cursorPromptText_);

    return cursorPromptText_;
}

void MachPromptText::clearCursorPromptText()
{
    CB_DEPIMPL(std::string, cursorPromptText_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);

    if (cursorPromptText_ != "")
    {
        cursorPromptText_ = "";
        promptDisplayed_ = NOTEXT;
    }
}

void MachPromptText::setCommandPromptText(const std::string& prompt)
{
    CB_DEPIMPL(std::string, commandPromptText_);
    CB_DEPIMPL(MachPromptText::PromptDisplayed, promptDisplayed_);
    CB_DEPIMPL(bool, refresh_);
    CB_DEPIMPL(bool, restartScroll_);
    CB_DEPIMPL(strings, commandPromptTextLines_);
    CB_DEPIMPL(GuiBmpFont, font_);

    if (commandPromptText_ != prompt)
    {
        commandPromptText_ = prompt;

        commandPromptTextLines_ = MachGuiMenuText::chopUpText(prompt, 2000, font_);

        if (promptDisplayed_ != CURSORTEXT)
        {
            refresh_ = true; // Force prompt text to be refreshed
            restartScroll_ = true;
        }
    }
}

const std::string& MachPromptText::commandPromptText() const
{
    CB_DEPIMPL(std::string, commandPromptText_);

    return commandPromptText_;
}

void MachPromptText::clearCommandPromptText()
{
    CB_DEPIMPL(std::string, commandPromptText_);
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
    CB_DEPIMPL(std::string, commandPromptText_);
    CB_DEPIMPL(std::string, cursorPromptText_);
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
    CB_DEPIMPL(std::string, chatMessageIntendedForStr_);
    CB_DEPIMPL(std::string, lastDisplayedChatMessage_);
    CB_DEPIMPL(int, beginningTextWidth_);

    // Update the caret
    update();

    std::string displayChatMessageStr = chatMessageIntendedForStr_ + text();
    int caretPosition = caretPos() + beginningTextWidth_;
    int startY = shadowFont_.height() / 2;
    if (Gui::uiScaleFactor() > 1)
        startY += 1;

    // Make sure we only update when the text has changed
    if (lastDisplayedChatMessage_ != displayChatMessageStr)
    {
        lastDisplayedChatMessage_ = displayChatMessageStr;

        Ren::Painter bmpPainter(promptBmp_);
        bmpPainter.clearRectangle(promptBmp_.size());

        // Work out starting position for text
        Ren::Point startBeginningText = Ren::Point(0, startY);
        Ren::Point shadowStartBeginningText = startBeginningText + Ren::Point(1, 1) * Gui::uiScaleFactor();
        Ren::Point startText = Ren::Point(caretPosition, startY);
        Ren::Point shadowStartText = startText + Ren::Point(1, 1) * Gui::uiScaleFactor();

        // Draw shadow and normal text
        bmpPainter.drawText(chatMessageIntendedForStr_, shadowStartBeginningText, shadowFont_, promptBmp_.width());
        bmpPainter.drawText(
            rightText(), shadowStartText, shadowFont_, promptBmp_.width() - caretPosition - 1 - beginningTextWidth_);
        bmpPainter.drawText(
            leftText(), shadowStartText, shadowFont_, caretPosition - beginningTextWidth_, Ren::AlignRight);
        bmpPainter.drawText(chatMessageIntendedForStr_, startBeginningText, font_, promptBmp_.width());
        bmpPainter.drawText(rightText(), startText, font_, promptBmp_.width() - caretPosition - beginningTextWidth_);
        bmpPainter.drawText(leftText(), startText, font_, caretPosition - beginningTextWidth_, Ren::AlignRight);
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
        Gui::Coord to = from + Gui::Vec(0, font_.height());
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

        Ren::Painter bmpPainter(pImpl_->promptBmp_);
        bmpPainter.clearRectangle(pImpl_->promptBmp_.size());

        // Render all the lines
        int startY = 0;
        if (textLines.size() == 1)
        {
            startY += pImpl_->shadowFont_.height() / 2;
            if (Gui::uiScaleFactor() > 1)
                startY += 1;
        }

        for (const std::string& line : textLines)
        {
            Ren::Point textPos(0, startY);
            Ren::Point shadowPos = textPos + Ren::Point(1, 1) * Gui::uiScaleFactor();
            bmpPainter.drawText(line, shadowPos, pImpl_->shadowFont_, pImpl_->promptBmp_.width() - shadowPos.x);
            bmpPainter.drawText(line, textPos, pImpl_->font_, pImpl_->promptBmp_.width());
            startY += pImpl_->shadowFont_.height() + 1 * Gui::uiScaleFactor();
        }

        if (pImpl_->restartScroll_)
        {
            pImpl_->blitToX_ = 0;
        }
    }

    // Blit text.
    GuiPainter::instance().blit(
        pImpl_->promptBmp_, Gui::Size(pImpl_->blitToX_, pImpl_->promptBmp_.height()), getPromptTextAbsolutePosition());

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
    CB_DEPIMPL(std::string, chatMessageIntendedForStr_);
    CB_DEPIMPL(int, beginningTextWidth_);
    CB_DEPIMPL(GuiBmpFont, shadowFont_);
    CB_DEPIMPL(int, opponentIndex_);
    CB_DEPIMPL(int, standardMessageIndex_);
    CB_DEPIMPL(MachInGameScreen*, pInGameScreen_);

    bool processed = false;

    if (event.state() == Gui::PRESSED)
    {
        static const auto& sendChatToSystemTrigger = MachGui::inputRegistry()->getBinds("chat-send-to-system"_bind);
        static const auto& showMenusTrigger = MachGui::inputRegistry()->getBinds("show-menus"_bind);

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
                        beginningTextWidth_ = shadowFont_.horizontalAdvance(chatMessageIntendedForStr_);
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
                beginningTextWidth_ = shadowFont_.horizontalAdvance(chatMessageIntendedForStr_);
            }
        }
        else if (sendChatToSystemTrigger.matches(event.keyWithMods()))
        {
            opponentIndex_ = SYSTEM_MESSAGE;
            enteringChatMessage_ = true;
            GuiManager::instance().charFocus(this);
            chatMessageIntendedForRace_ = MachPhys::N_RACES;

            GuiResourceString sendToSystemStr(IDS_SENDTOSYSTEM);
            chatMessageIntendedForStr_ = sendToSystemStr.asString();
            beginningTextWidth_ = shadowFont_.horizontalAdvance(chatMessageIntendedForStr_);
        }
        else if (
            event.key() == Device::KeyCode::ESCAPE || event.key() == Device::KeyCode::ENTER
            || showMenusTrigger.matches(event.keyWithMods()))
        {
            if (enteringChatMessage_)
            {
                // Send a chat message
                if (event.key() == Device::KeyCode::ENTER)
                {
                    submit();
                }

                // Clear message from prompt text
                enteringChatMessage_ = false;
                GuiManager::instance().removeCharFocus();
                setText({});
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

void MachPromptText::submit()
{
    CB_DEPIMPL_AUTO(chatMessageIntendedForRace_);
    CB_DEPIMPL_AUTO(chatMessageIntendedForStr_);
    CB_DEPIMPL_AUTO(opponentIndex_);
    CB_DEPIMPL_AUTO(pInGameScreen_);

    if (opponentIndex_ == SYSTEM_MESSAGE)
    {
        if (strncasecmp(text().c_str(), "MUSIC", 5) == 0 && text().length() > 5)
        {
            int track = atoi(&text().c_str()[5]);

            if (track > 0 && // Valid track number entered
                DevCD::instance().isAudioCDPresent() && // Audio CD is in CD-Rom
                track < DevCD::instance().numberOfTracks()) // Track number is not outside the number of tracks on CD
            {
                DevCD::instance().play(track);
            }
        }
        else if (strcasecmp(text().c_str(), "EXIT") == 0) // Instant Exit
        {
            pInGameScreen_->instantExit(true);
        }

        MachGuiInGameChatMessages::instance().addMessage(chatMessageIntendedForStr_ + text());
    }
    else
    {
        std::string chatMessageStr;

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
}

bool MachPromptText::processesMouseEvents() const
{
    return false;
}
