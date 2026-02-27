/*
 * E D I T B O X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/SingleLineEditBox.hpp"

#include "base/Diag.hpp"
#include "device/ButtonEvent.hpp"

MachGuiSingleLineEditBox::MachGuiSingleLineEditBox(GuiDisplayable* pParent, const Gui::Box& box, const GuiBmpFont& font)
    : GuiSingleLineEditBox(pParent, box, font)
    , ignoreSpaceAtBeginning_(true)
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    TEST_INVARIANT;
}

MachGuiSingleLineEditBox::MachGuiSingleLineEditBox(
    GuiDisplayable* pParent, const Gui::Box& box, const Ren::Font& font, const Ren::TextOptions& options)
    : GuiSingleLineEditBox(pParent, box, font, options)
    , ignoreSpaceAtBeginning_(true)
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    TEST_INVARIANT;
}

MachGuiSingleLineEditBox::~MachGuiSingleLineEditBox()
{

    TEST_INVARIANT;
}

// virtual
void MachGuiSingleLineEditBox::drawBackground()
{
}

void MachGuiSingleLineEditBox::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiSingleLineEditBox& t)
{

    o << "MachGuiSingleLineEditBox " << reinterpret_cast<void*>(const_cast<MachGuiSingleLineEditBox*>(&t)) << " start"
      << std::endl;
    o << "MachGuiSingleLineEditBox " << reinterpret_cast<void*>(const_cast<MachGuiSingleLineEditBox*>(&t)) << " end"
      << std::endl;

    return o;
}

// virtual
bool MachGuiSingleLineEditBox::doHandleCharEvent(const GuiCharEvent& e)
{
    DEBUG_STREAM(
        DIAG_NEIL,
        "MachGuiSingleLineEditBox::doHandleCharEvent " << e.getChar() << " " << static_cast<int>(e.getChar())
                                                       << std::endl);

    if (!isEnabled())
        return false;

    if (ignoreSpaceAtBeginning_ && leftText().empty() && e.getChar() == ' ')
    {
        return true;
    }

    if (clearTextOnNextChar_)
    {
        setText("");
        clearTextOnNextChar_ = false;
    }

    return GuiSingleLineEditBox::doHandleCharEvent(e);
}

void MachGuiSingleLineEditBox::clearTextOnNextChar(bool newVal)
{
    clearTextOnNextChar_ = newVal;
}

bool MachGuiSingleLineEditBox::clearTextOnNextChar() const
{
    return clearTextOnNextChar_;
}

void MachGuiSingleLineEditBox::ignoreSpaceAtBeginning(bool ignore)
{
    ignoreSpaceAtBeginning_ = ignore;
}

/* End EDITBOX.CPP **************************************************/
