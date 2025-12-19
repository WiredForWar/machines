/*
 * M E N U T E X T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "MenuText.hpp"

#include "gui/font.hpp"
#include "gui/gui.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "render/Font.hpp"
#include "render/TextOptions.hpp"

#include <cstring>

void MachGuiMenuText::chopUpText(const std::string& text, size_t maxWidth, const GuiBmpFont& font, strings* pStrings)
{
    // WARNING : this function breaks down if the width is not big enough to store at least 6 characters.

    std::string choppedUpText;
    int charPos = 0;
    int textLength = text.length();
    std::size_t curWidth = 0;
    bool beginningOfLine = true;

    while (charPos < textLength)
    {
        const char curChar = text[charPos];

        // Ignore spaces at beginning of line
        if (beginningOfLine && curChar == ' ')
        {
        }
        // Force new line
        else if (curChar == '\n')
        {
            // Reset beginningOfLine flag
            beginningOfLine = true;
            pStrings->push_back(choppedUpText);
            choppedUpText = "";
            curWidth = 0;
        }
        // Is text gonna be too wide if we add this character?
        else if (curWidth + font.charWidth(curChar) + font.spacing() > maxWidth)
        {
            // Get pointer to character ten position from end of string.
            const char* almostStrEnd = choppedUpText.c_str();
            if (choppedUpText.length() > 11)
                almostStrEnd = &choppedUpText.c_str()[choppedUpText.length() - 11];

            // Add part of text to pStrings...

            // Do we have a nice break, i.e. a space character, therefore not having to
            // chop a word in two...
            if (curChar == ' ')
            {
                pStrings->push_back(choppedUpText);
                choppedUpText = "";
                curWidth = 0;

                // Reset beginningOfLine flag
                beginningOfLine = true;
            }
            // Does space exist in last ten characters? If it does it's worth moving the
            // beginning of the word onto the next line...
            else if (std::strchr(almostStrEnd, ' '))
            {
                // Remove beginning of last word from this line and stuff on next line...
                std::string newLine;
                curWidth = 0;
                // Remove last chars until we find a space
                while (choppedUpText[choppedUpText.length() - 1] != ' ')
                {
                    char lastChar = choppedUpText[choppedUpText.length() - 1];
                    newLine = lastChar + newLine;
                    curWidth += font.charWidth(lastChar) + font.spacing();
                    choppedUpText.erase(choppedUpText.length() - 1, 1);
                }
                newLine += curChar;

                curWidth += font.charWidth(curChar) + font.spacing();
                pStrings->push_back(choppedUpText);
                choppedUpText = newLine;
                // We are putting stuff onto the next line therefore the beginningOfLine flag should not be reset.
                beginningOfLine = false;
            }
            else
            {
                // Splitting word across two lines...
                std::string newLine;
                std::size_t hyphenWidth = font.charWidth('-') + font.spacing();
                curWidth = 0;

                // Remove as many characters as necessary so that a hyphen can be
                // added at the end of the line.
                while (curWidth < hyphenWidth)
                {
                    char lastChar = choppedUpText[choppedUpText.length() - 1];
                    newLine = lastChar + newLine;
                    curWidth += font.charWidth(lastChar) + font.spacing();
                    // choppedUpText.remove( choppedUpText.length() - 1 );
                    choppedUpText.erase(choppedUpText.length() - 1, 1);
                }

                choppedUpText += '-';
                pStrings->push_back(choppedUpText);
                // Start of next line
                choppedUpText = newLine;
                choppedUpText += curChar;
                curWidth += font.charWidth(curChar) + font.spacing();

                // We are putting stuff onto the next line therefore the beginningOfLine flag should not be reset.
                beginningOfLine = false;
            }
        }
        else
        {
            // We CAN add this character without overflowing maxWidth...
            choppedUpText += curChar;
            curWidth += font.charWidth(curChar) + font.spacing();

            beginningOfLine = false;
        }

        ++charPos;
    }

    if (choppedUpText.length() != 0)
    {
        pStrings->push_back(choppedUpText);
    }
}

void MachGuiMenuText::chopUpText(
    const std::string& text,
    size_t maxWidth,
    const Render::Font& font,
    const Render::TextOptions& options,
    strings* pStrings)
{
    std::string choppedUpText;
    int charPos = 0;
    const int textLength = static_cast<int>(text.length());
    std::size_t curWidth = 0;
    bool beginningOfLine = true;

    while (charPos < textLength)
    {
        const char curChar = text[charPos];

        // Ignore spaces at beginning of line
        if (beginningOfLine && curChar == ' ')
        {
        }
        // Force new line
        else if (curChar == '\n')
        {
            // Reset beginningOfLine flag
            beginningOfLine = true;
            pStrings->push_back(choppedUpText);
            choppedUpText = "";
            curWidth = 0;
        }
        // Is text gonna be too wide if we add this character?
        else if (curWidth + font.charWidth(curChar) + options.letterSpacing() > maxWidth)
        {
            // Get pointer to character ten position from end of string.
            const char* almostStrEnd = choppedUpText.c_str();
            if (choppedUpText.length() > 11)
                almostStrEnd = &choppedUpText.c_str()[choppedUpText.length() - 11];

            // Add part of text to pStrings...

            // Do we have a nice break, i.e. a space character, therefore not having to
            // chop a word in two...
            if (curChar == ' ')
            {
                pStrings->push_back(choppedUpText);
                choppedUpText = "";
                curWidth = 0;

                // Reset beginningOfLine flag
                beginningOfLine = true;
            }
            // Does space exist in last ten characters? If it does it's worth moving the
            // beginning of the word onto the next line...
            else if (std::strchr(almostStrEnd, ' '))
            {
                // Remove beginning of last word from this line and stuff on next line...
                std::string newLine;
                curWidth = 0;
                // Remove last chars until we find a space
                while (choppedUpText[choppedUpText.length() - 1] != ' ')
                {
                    const char lastChar = choppedUpText[choppedUpText.length() - 1];
                    newLine = lastChar + newLine;
                    curWidth += font.charWidth(lastChar) + options.letterSpacing();
                    choppedUpText.erase(choppedUpText.length() - 1, 1);
                }
                newLine += curChar;

                curWidth += font.charWidth(curChar) + options.letterSpacing();
                pStrings->push_back(choppedUpText);
                choppedUpText = newLine;
                // We are putting stuff onto the next line therefore the beginningOfLine flag should not be reset.
                beginningOfLine = false;
            }
            else
            {
                // Splitting word across two lines...
                std::string newLine;
                const std::size_t hyphenWidth = static_cast<std::size_t>(font.horizontalAdvance(std::string(1, '-')));
                curWidth = 0;

                // Remove as many characters as necessary so that a hyphen can be
                // added at the end of the line.
                while (curWidth < hyphenWidth)
                {
                    const char lastChar = choppedUpText[choppedUpText.length() - 1];
                    newLine = lastChar + newLine;
                    curWidth += font.charWidth(lastChar) + options.letterSpacing();
                    choppedUpText.erase(choppedUpText.length() - 1, 1);
                }

                choppedUpText += '-';
                pStrings->push_back(choppedUpText);
                // Start of next line
                choppedUpText = newLine;
                choppedUpText += curChar;
                curWidth += font.charWidth(curChar) + options.letterSpacing();

                // We are putting stuff onto the next line therefore the beginningOfLine flag should not be reset.
                beginningOfLine = false;
            }
        }
        else
        {
            // We CAN add this character without overflowing maxWidth...
            choppedUpText += curChar;
            curWidth += font.charWidth(curChar) + options.letterSpacing();

            beginningOfLine = false;
        }

        ++charPos;
    }

    if (choppedUpText.length() != 0)
    {
        pStrings->push_back(choppedUpText);
    }
}

MachGuiMenuText::MachGuiMenuText(
    GuiDisplayable* pParent,
    const Gui::Box& box,
    const ResolvedUiString& str,
    const SysPathName& bitmapFontPath,
    Gui::Alignment alignment)
    : GuiDisplayable(pParent, box)
    , bitmapFontPath_(bitmapFontPath)
    , alignment_(alignment)
{
    strings_.reserve(128);
    const GuiBmpFont &font = GuiBmpFont::getFont(bitmapFontPath_);
    fontHeight_ = font.height();

    chopUpText(str, width(), font, &strings_);

    TEST_INVARIANT;
}

MachGuiMenuText::MachGuiMenuText(
    GuiDisplayable* pParent,
    const Gui::Box& box,
    const ResolvedUiString& str,
    const Render::Font& font,
    const Render::TextOptions& options,
    Gui::Alignment alignment)
    : GuiDisplayable(pParent, box)
    , font_(&font)
    , textOptions_(options)
    , alignment_(alignment)
{
    strings_.reserve(128);
    fontHeight_ = font.height();

    chopUpText(str, width(), font, textOptions_, &strings_);

    TEST_INVARIANT;
}

MachGuiMenuText::~MachGuiMenuText()
{
    TEST_INVARIANT;
}

void MachGuiMenuText::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMenuText& t)
{

    o << "MachGuiMenuText " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMenuText " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiMenuText::doDisplay()
{
    size_t totalHeight = (strings_.size() * (fontHeight_ + 1 * MachGui::menuScaleFactor())) - 1 * MachGui::menuScaleFactor();

    ASSERT_INFO(totalHeight);
    ASSERT_INFO(height());
    ASSERT(totalHeight <= height(), "height required to render text is greater than the height assigned");

    int startY = absoluteBoundary().minCorner().y();
    Gui::Alignment verticalAlignment = alignment_ & Gui::AlignVertical_Mask;
    if (!verticalAlignment) {
        // Vertical center is the default behavior
        verticalAlignment = Gui::AlignVCenter;
    }

    if (verticalAlignment & Gui::AlignTop)
        ;
    else if (verticalAlignment & Gui::AlignVCenter)
        startY += (height() - totalHeight) / 2.0;
    else if (verticalAlignment & Gui::AlignBottom)
        startY += height() - totalHeight;

    auto drawText = [&](auto textWidthCb, auto drawTextCb) {
        for (std::size_t i = 0; i < strings_.size(); ++i)
        {
            int textWidth = textWidthCb(strings_[i]);
            int textX = 0;

            if (alignment_ & Gui::AlignHCenter)
                textX = absoluteBoundary().minCorner().x() + ((width() - textWidth) / 2.0);
            else if (alignment_ & Gui::AlignRight)
                textX = absoluteBoundary().minCorner().x() + width() - textWidth;
            else if (alignment_ & Gui::AlignLeft)
                textX = absoluteBoundary().minCorner().x();

            int textY = startY + (i * (fontHeight_ + 1 * MachGui::menuScaleFactor()));

            drawTextCb(Gui::Coord(textX, textY), strings_[i]);
        }
    };

    if (font_)
    {
        const Render::Font& font = *font_;
        auto textWidthCb = [&](const std::string& text) -> int { return font.horizontalAdvance(text, textOptions_); };
        auto drawTextCb = [&](const Gui::Coord& coord, const std::string& text)
        {
            IGuiPainter& p = GuiPainter::instance();
            p.drawText(coord, text, textOptions_, font);
        };

        drawText(textWidthCb, drawTextCb);
    }
    else
    {
        const GuiBmpFont font(GuiBmpFont::getFont(bitmapFontPath_));

        auto textWidthCb = [&font](const std::string& text) -> int { return font.horizontalAdvance(text); };
        auto drawTextCb
            = [&font](const Gui::Coord& coord, const std::string& text) { font.drawText(text, coord, 1000); };

        drawText(textWidthCb, drawTextCb);
    }
}
