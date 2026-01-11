/*
 * E D I T B O X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "gui/editbox.hpp"
#include "gui/painter.hpp"
#include "gui/event.hpp"
#include "gui/manager.hpp"
#include "device/time.hpp"
#include "device/butevent.hpp"
#include "render/Font.hpp"

GuiSingleLineEditBox::GuiSingleLineEditBox(GuiDisplayable* pParent, const Gui::Box& box)
    : GuiDisplayable(pParent, box)
    , showCaret_(true)
    , backgroundColour_(Gui::BLACK())
    , caretColour_(Gui::WHITE())
    , dontUpdate_(true)
    , borderColour_(Gui::WHITE())
{
}

GuiSingleLineEditBox::GuiSingleLineEditBox(
    GuiDisplayable* pParent,
    const Gui::Box& box,
    const Ren::Font& font,
    const Ren::TextOptions& options)
    : GuiSingleLineEditBox(pParent, box)
{
    ttfFont_ = &font;
    textOptions_ = options;
}

GuiSingleLineEditBox::GuiSingleLineEditBox(GuiDisplayable* pParent, const Gui::Box& box, const GuiBmpFont& font)
    : GuiSingleLineEditBox(pParent, box)
{
    font_ = font;
    TEST_INVARIANT;
}

GuiSingleLineEditBox::~GuiSingleLineEditBox()
{
    TEST_INVARIANT;
}

void GuiSingleLineEditBox::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const GuiSingleLineEditBox& t)
{

    o << "GuiSingleLineEditBox " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "GuiSingleLineEditBox " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void GuiSingleLineEditBox::updateTextViews()
{
    if (cursorIndex_ > text_.length())
        cursorIndex_ = text_.length();

    std::string_view fullView(text_);
    leftTextView_ = fullView.substr(0, cursorIndex_);
    rightTextView_ = fullView.substr(cursorIndex_);

    if (ttfFont_)
        caretPos_ = ttfFont_->horizontalAdvance(leftTextView_, textOptions_);
    else
        caretPos_ = font_.horizontalAdvance(leftTextView_);

    const int maxCaret = maxWidth() - (border_ ? 5 : 1);
    if (caretPos_ > maxCaret)
        caretPos_ = maxCaret;
}

// virtual
void GuiSingleLineEditBox::doDisplay()
{
    drawBackground();

    int offset = border_ ? 2 * Gui::uiScaleFactor() : 0;

    Gui::Coord startText = Gui::Coord(
        absoluteBoundary().minCorner().x() + offset,
        absoluteBoundary().minCorner().y() + offset);

    GuiPainter& painter = GuiPainter::instance();

    if (ttfFont_)
    {
        painter.drawText(startText, text(), textOptions_, *ttfFont_);
    }
    else
    {
        painter.drawText(text(), Ren::Point(startText.x(), startText.y()), font_, maxWidth());
    }

    if (GuiManager::instance().charFocusExists() && &GuiManager::instance().charFocus() == this
        && showCaret_) // Only show caret if we have focus
    {
        GuiPainter::instance().line(
            Gui::Coord(
                absoluteBoundary().minCorner().x() + caretPos_ + offset,
                absoluteBoundary().minCorner().y() + offset),
            Gui::Coord(
                absoluteBoundary().minCorner().x() + caretPos_ + offset,
                absoluteBoundary().maxCorner().y() - offset),
            caretColour_,
            1 * Gui::uiScaleFactor());
    }

    if (border_)
    {
        GuiPainter::instance().hollowRectangle(absoluteBoundary(), borderColour_, 1 * Gui::uiScaleFactor());
    }
}

void GuiSingleLineEditBox::update()
{
    double time = DevTime::instance().time();
    int itime = time * 2;

    bool newShowCaret = itime % 2;
    if (! dontUpdate_ && newShowCaret != showCaret_)
    {
        showCaret_ = newShowCaret;
        changed();
    }

    dontUpdate_ = false;
}

// virtual
bool GuiSingleLineEditBox::doHandleCharEvent(const GuiCharEvent& e)
{
    // Check that char is usable
    if (ttfFont_)
    {
        if (ttfFont_->charWidth(e.getChar()) <= 0)
            return false;
    }
    else
    {
        if (font_.charWidth(e.getChar()) <= 0)
            return false;
    }

    // Check to see if adding this character is allowed, i.e. length of string will
    // be less than or equal to maxChars_ ( maximum length of string allowed ).
    if (text_.length() < maxChars_ || maxChars_ == 0)
    {
        text_.insert(cursorIndex_, 1, e.getChar());
        ++cursorIndex_;

        onTextChanged();
    }

    return true;
}

// virtual
bool GuiSingleLineEditBox::doHandleKeyEvent(const GuiKeyEvent& e)
{
    bool processed = false;

    if (e.buttonEvent().action() == DevButtonEvent::PRESS)
    {
        processed = true;
        switch (e.buttonEvent().scanCode())
        {
            case Device::KeyCode::LEFT_ARROW:
                leftArrowEvent();
                break;
            case Device::KeyCode::RIGHT_ARROW:
                rightArrowEvent();
                break;
            case Device::KeyCode::BACK_SPACE:
                backspaceEvent();
                break;
            case Device::KeyCode::DELETE:
                deleteEvent();
                break;
            case Device::KeyCode::HOME:
                homeEvent();
                break;
            case Device::KeyCode::END:
                endEvent();
                break;
            default:
                processed = false;
                break;
        }
    }

    return processed;
}

void GuiSingleLineEditBox::setCursorPosition(std::size_t position)
{
    if (position > text_.length())
        position = text_.length();

    if (position == cursorIndex_)
        return;

    cursorIndex_ = position;
    updateTextViews();
    forceRedraw();
}

void GuiSingleLineEditBox::leftArrowEvent()
{
    if (cursorIndex_ > 0)
        setCursorPosition(cursorIndex_ - 1);
}

void GuiSingleLineEditBox::rightArrowEvent()
{
    setCursorPosition(cursorIndex_ + 1);
}

void GuiSingleLineEditBox::homeEvent()
{
    setCursorPosition(0);
}

void GuiSingleLineEditBox::endEvent()
{
    setCursorPosition(text_.length());
}

void GuiSingleLineEditBox::backspaceEvent()
{
    // Check to see if were not already at the beginning of the edit box
    if (cursorIndex_ > 0)
    {
        char c = text_[cursorIndex_ - 1];
        text_.erase(cursorIndex_ - 1, 1);
        --cursorIndex_;

        onTextChanged();
    }
}

void GuiSingleLineEditBox::deleteEvent()
{
    // Check to see if were not already at the end of the edit box
    if (cursorIndex_ < text_.length())
    {
        text_.erase(cursorIndex_, 1);
        onTextChanged();
    }
}

void GuiSingleLineEditBox::onTextChanged()
{
    updateTextViews();
    forceRedraw();

    if (textChangedCallback_)
    {
        textChangedCallback_(this);
    }
}

void GuiSingleLineEditBox::forceRedraw()
{
    // Redraw edit box
    showCaret_ = true;
    dontUpdate_ = true;
    changed();
}

void GuiSingleLineEditBox::setText(const std::string& newText)
{
    PRE(maxChars_ ? newText.length() <= maxChars_ : true);

    text_ = newText;
    cursorIndex_ = 0;

    onTextChanged();
}

std::string GuiSingleLineEditBox::text() const
{
    return text_;
}

// virtual
void GuiSingleLineEditBox::drawBackground()
{
    GuiPainter::instance().filledRectangle(absoluteBoundary(), backgroundColour_);
}

void GuiSingleLineEditBox::maxChars(size_t newMaxChars)
{
    maxChars_ = newMaxChars;
}

void GuiSingleLineEditBox::backgroundColour(const GuiColour& colour)
{
    backgroundColour_ = colour;
}

void GuiSingleLineEditBox::caretColour(const GuiColour& colour)
{
    caretColour_ = colour;
}

void GuiSingleLineEditBox::clear()
{
    setText({});
}

void GuiSingleLineEditBox::setTextChangedCallback(Callback callback)
{
    textChangedCallback_ = callback;
}

void GuiSingleLineEditBox::border(bool b)
{
    border_ = b;
}

void GuiSingleLineEditBox::borderColour(const GuiColour& colour)
{
    borderColour_ = colour;
}

// virtual
void GuiSingleLineEditBox::doHandleMouseClickEvent(const GuiMouseEvent&)
{
    GuiManager::instance().charFocus(this);
}

std::string_view GuiSingleLineEditBox::leftText() const
{
    return leftTextView_;
}

std::string_view GuiSingleLineEditBox::rightText() const
{
    return rightTextView_;
}

Gui::XCoord GuiSingleLineEditBox::caretPos() const
{
    return caretPos_;
}

bool GuiSingleLineEditBox::showCaret() const
{
    return showCaret_;
}

const GuiColour& GuiSingleLineEditBox::caretColour() const
{
    return caretColour_;
}

// virtual
int GuiSingleLineEditBox::maxWidth() const
{
    return width();
}

/* End EDITBOX.CPP **************************************************/
