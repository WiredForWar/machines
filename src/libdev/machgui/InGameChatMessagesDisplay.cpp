/*
 * C H A T M S G D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/InGameChatMessagesDisplay.hpp"
#include "machgui/InGameChatMessages.hpp"
#include "machgui/gui.hpp"
#include "gui/Font.hpp"
#include "gui/GuiPainter.hpp"
#include "render/Painter.hpp"
#include "gui/Event.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "render/Device.hpp"
#include "render/Surface.hpp"
#include "ctl/List.hpp"

MachGuiInGameChatMessagesDisplay::MachGuiInGameChatMessagesDisplay(GuiDisplayable* pParent, const Gui::Box& relBox)
    : GuiDisplayable(pParent, relBox, GuiDisplayable::LAYER4)
    , font_(Gui::getFont(MachGui::getScaledImagePath("gui/menu/promtfnt.bmp")))
    , shadowFont_(Gui::getFont(MachGui::getScaledImagePath("gui/menu/promdfnt.bmp")))
{
    textBmp_ = RenSurface::createAnonymousSurface(Ren::Size(width(), height()));
    textBmp_.enableColourKeying();

    redrawEveryFrame(true);

    TEST_INVARIANT;
}

MachGuiInGameChatMessagesDisplay::~MachGuiInGameChatMessagesDisplay()
{
    TEST_INVARIANT;
}

void MachGuiInGameChatMessagesDisplay::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiInGameChatMessagesDisplay& t)
{
    o << "MachGuiInGameChatMessagesDisplay " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiInGameChatMessagesDisplay " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiInGameChatMessagesDisplay::doDisplay()
{
    bool needsUpdate = MachGuiInGameChatMessages::instance().update();

    if ((needsUpdate && !MachGuiInGameChatMessages::instance().messages().empty()) || forceUpdate_)
    {
        forceUpdate_ = false;

        // Reset bitmap to transparent
        Ren::Painter(textBmp_).clearRectangle(textBmp_.size());

        // Render up to five lines of chat messages
        int fontHeight = shadowFont_.height() + 1;
        int index = 0;

        Ren::Painter bmpPainter(textBmp_);
        for (ctl_list<std::string>::const_iterator iter = MachGuiInGameChatMessages::instance().messages().begin();
             iter != MachGuiInGameChatMessages::instance().messages().end();
             ++iter)
        {
            bmpPainter.drawText(*iter, Ren::Point(1, 1 + (fontHeight * index)), shadowFont_, width());
            bmpPainter.drawText(*iter, Ren::Point(0, fontHeight * index), font_, width());
            ++index;
        }
    }

    if (MachGuiInGameChatMessages::instance().messages().size() > 0)
    {
        // Blit text.
        GuiPainter::instance().blit(textBmp_, absoluteBoundary().minCorner());
    }
}

void MachGuiInGameChatMessagesDisplay::forceUpdate()
{
    forceUpdate_ = true;
}

void MachGuiInGameChatMessagesDisplay::setPassEventsTo(GuiDisplayable* pPassEventsTo)
{
    pPassEventsTo_ = pPassEventsTo;
}

/* NA 30/11/98. New processesMouseEvents added to GuiDisplayable means I don't need these
//virtual
void MachGuiInGameChatMessagesDisplay::doHandleMouseClickEvent( const GuiMouseEvent& rel )
{
    GuiMouseEvent relToOther = rel;

    if ( passEventTo( &relToOther ) )
    {
        pPassEventsTo_->doHandleMouseClickEvent( relToOther );
    }
}

//virtual
void MachGuiInGameChatMessagesDisplay::doHandleMouseEnterEvent( const GuiMouseEvent& rel )
{
    GuiMouseEvent relToOther = rel;

    if ( passEventTo( &relToOther ) )
    {
        pPassEventsTo_->doHandleMouseEnterEvent( relToOther );
    }
}

//virtual
void MachGuiInGameChatMessagesDisplay::doHandleMouseExitEvent( const GuiMouseEvent& rel )
{
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
void MachGuiInGameChatMessagesDisplay::doHandleContainsMouseEvent( const GuiMouseEvent& rel )
{
    GuiMouseEvent relToOther = rel;

    if ( passEventTo( &relToOther ) )
    {
        pPassEventsTo_->doHandleContainsMouseEvent( relToOther );
    }
}

bool MachGuiInGameChatMessagesDisplay::passEventTo( GuiMouseEvent* pMouseEvent )
{
    bool returnValue = false;

    if ( pPassEventsTo_ )
    {
        Gui::Coord absEventPos = absoluteBoundary().minCorner();
        absEventPos.x( absEventPos.x() + pMouseEvent->coord().x() );
        absEventPos.y( absEventPos.y() + pMouseEvent->coord().y() );

        if ( pPassEventsTo_->contains( absEventPos ) )
        {
            pMouseEvent->translate( Gui::Vec( ( absEventPos.x() - pPassEventsTo_->absoluteCoord().x() ) -
pMouseEvent->coord().x(), ( absEventPos.y() - pPassEventsTo_->absoluteCoord().y() ) - pMouseEvent->coord().y() ) );
            returnValue = true;
        }
    }

    return returnValue;
} */

// virtual
bool MachGuiInGameChatMessagesDisplay::processesMouseEvents() const
{
    return false;
}

/* End CHATMSGD.CPP *************************************************/
