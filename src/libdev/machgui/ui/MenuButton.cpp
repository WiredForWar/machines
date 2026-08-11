/*
 * M E N U B U T N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "MenuButton.hpp"

#include "machgui/gui.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "sound/Mixer.hpp"
#include "sound/WaveformId.hpp"
#include "sound/SampleParameters.hpp"
#include "gui/Event.hpp"
#include "gui/GuiPainter.hpp"
#include "gui/ResourceString.hpp"
#include "machgui/internal/SoundManager.hpp"
#include "render/Font.hpp"
#include "render/TextOptions.hpp"
#include "render/Device.hpp"
#include "system/PathName.hpp"
#include "system/WindowsAPI.hpp"

using namespace MachGui;

MachGuiMenuButton::MachGuiMenuButton(
    GuiRoot* pRootParent,
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    unsigned int stringId,
    MachGui::ButtonEvent buttonEvent)
    : GuiDisplayable(pParent, box)
    , pRootParent_(pRootParent)
    , pStartupScreens_(pParent)
    , stringId_(stringId)
    , buttonEvent_(buttonEvent)
{
    setAcceptsFocus(true);

    TEST_INVARIANT;
}

MachGuiMenuButton::MachGuiMenuButton(
    MachGuiStartupScreens* pStartupScreens,
    const Gui::Box& box,
    unsigned int stringId,
    MachGui::ButtonEvent buttonEvent,
    GuiDisplayable* pParent)
    : GuiDisplayable(pParent, box)
    , pStartupScreens_(pStartupScreens)
    , stringId_(stringId)
    , buttonEvent_(buttonEvent)
{
    setAcceptsFocus(true);

    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    TEST_INVARIANT;
}

MachGuiMenuButton::~MachGuiMenuButton()
{
    TEST_INVARIANT;
}

void MachGuiMenuButton::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMenuButton& t)
{

    o << "MachGuiMenuButton " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMenuButton " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiMenuButton::doHandleMouseClickEvent(const GuiMouseEvent& rel)
{
    if (!isEnabled())
    {
        // Can not press button when disabled. Play discouraging sound...
        if (rel.leftButton() == Gui::RELEASED)
        {
            MachGuiSoundManager::instance().playSound("gui/sounds/clickbad.wav");
        }

        return;
    }

    if (rel.leftButton() == Gui::PRESSED)
    {
        flash_ = true;
        changed();
    }
    else
    {
        flash_ = false;
        changed();
        pStartupScreens_->buttonAction(buttonEvent_);
    }
}

// virtual
void MachGuiMenuButton::doHandleMouseExitEvent(const GuiMouseEvent& /*rel*/)
{
    if (!isEnabled())
        return;

    highlighted_ = false;
    flash_ = false;
    changed();
}

// virtual
void MachGuiMenuButton::doHandleMouseEnterEvent(const GuiMouseEvent& /*rel*/)
{
    if (!isEnabled())
        return;

    // Set up the sound to be played
    MachGuiSoundManager::instance().playSound("gui/sounds/highligh.wav");

    highlighted_ = true;
    changed();
}

// virtual
void MachGuiMenuButton::doDisplay()
{
    static uint glowWidth = MachGui::buttonGlowBmp().width();
    static uint glowHeight = MachGui::buttonGlowBmp().height();

    ASSERT(glowWidth >= width(), "glow bitmap not wide enough");
    ASSERT(glowHeight >= height(), "glow bitmap not high enough");
    ASSERT(MachGui::buttonDisableBmp().width() >= width(), "disable bitmap not wide enough");
    ASSERT(MachGui::buttonDisableBmp().height() >= height(), "disable bitmap not high enough");

    uint glowX = (glowWidth - width()) / 2.0;
    uint glowY = (glowHeight - height()) / 2.0;

    GuiPainter& p = GuiPainter::instance();
    // Draw background to button ( glow or normal backdrop ).
    if (flash_ || highlighted_)
    {
        p.blit(MachGui::buttonGlowBmp(), Gui::Box(Gui::Coord(glowX, glowY), size()), absoluteBoundary().minCorner());
    }

    GuiResourceString str(stringId_);
    std::string text = str.asString();

    Ren::TextOptions options = MachGui::Menu::menuLightTextOptions();

    if (flash_ || highlighted_)
    {
        options.setColor(Menu::highlightedTextColor());
        options.setOutline(1 * MachGui::menuScaleFactor(), Gui::WHITE());
    }
    else if (isFocusControl())
    {
        options.setColor(Menu::focusedTextColor());
    }

    if (flash_)
    {
        options.setUnderline(true);
    }

    const Ren::Font& font = MachGui::Menu::font();
    std::size_t textWidth = font.horizontalAdvance(text, options);
    std::size_t textHeight = font.height();

    size_t textX = absoluteBoundary().minCorner().x() + (width() - textWidth) / 2.0;
    size_t textY = absoluteBoundary().minCorner().y() + (height() - textHeight) / 2.0;

    p.drawText(Gui::Coord(textX, textY), text, options, font);

    // Show disabled button if necessary
    if (!isEnabled())
    {
        p.blit(MachGui::buttonDisableBmp(), Gui::Box(Gui::Coord{}, size()), absoluteBoundary().minCorner());
    }
}

bool MachGuiMenuButton::highlighted() const
{
    return highlighted_;
}

uint MachGuiMenuButton::stringId() const
{
    return stringId_;
}

void MachGuiMenuButton::doEnabledChanged()
{
    flash_ = false;
    highlighted_ = false;
}

// virtual
bool MachGuiMenuButton::executeControl()
{
    flash_ = true;

    // Refresh button
    changed();

    // Initiate button action
    pStartupScreens_->buttonAction(buttonEvent_);

    return true;
}

// virtual
void MachGuiMenuButton::hasFocus(bool newValue)
{
    GuiDisplayable::hasFocus(newValue);

    changed();
}

/* End MENUBUTN.CPP *************************************************/
