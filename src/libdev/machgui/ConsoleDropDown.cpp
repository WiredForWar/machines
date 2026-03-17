#include "ConsoleDropDown.hpp"

#include "gui/Event.hpp"
#include "gui/GuiPainter.hpp"
#include "gui/Manager.hpp"
#include "machgui/gui.hpp"

#include "system/IConsole.hpp"
#include "utility/String.hpp"

#include "gui/Clipboard.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace
{

constexpr const char kConsoleFontPath[] = "gui/menu/promtfnt.bmp";

} // namespace

MachGuiConsoleDropDown::MachGuiConsoleDropDown(GuiDisplayable* parent) :
    GuiDisplayable(parent, GuiDisplayable::LAYER5)
{
    redrawEveryFrame(true);

    borderThickness_ = static_cast<int>(4 * Gui::uiScaleFactor());
    inputPadding_ = static_cast<int>(8 * Gui::uiScaleFactor());

    inputFont_ = Gui::getFont(MachGui::getScaledImagePath(kConsoleFontPath));
    inputAreaHeight_ = static_cast<int>(inputFont_.height()) + 2 * inputPadding_ + borderThickness_;

    inputBox_ = std::make_unique<MachGuiSingleLineEditBox>(this, Gui::Box{}, inputFont_);
    inputBox_->backgroundColour(MachGui::CONSOLEFILLCOLOR());
    inputBox_->border(false);
    inputBox_->ignoreSpaceAtBeginning(false);
    inputBox_->maxChars(256);
}

MachGuiConsoleDropDown::~MachGuiConsoleDropDown() = default;

void MachGuiConsoleDropDown::setConsole(System::IConsole* console)
{
    if (pConsole_ == console)
    {
        return;
    }

    outputListenerHandle_.reset();

    pConsole_ = console;

    if (pConsole_ != nullptr)
    {
        outputListenerHandle_ = pConsole_->addOutputListener([this](std::string_view) { updateOutputFromConsole(); });
    }

    updatePromptSurface(pConsole_ ? pConsole_->prompt() : std::string_view{});

    if (width() && height())
        doLayout();

    updateOutputFromConsole();
}

void MachGuiConsoleDropDown::setViewportSize(Gui::Size size)
{
    size.setHeight(size.height() / 3);
    setRelativeBoundary(Gui::Box(0, 0, size.width(), size.height()));
    doLayout();
    updateOutputFromConsole();
}

void MachGuiConsoleDropDown::submit()
{
    if (pConsole_ != nullptr)
    {
        pConsole_->submit(inputText());
    }
    inputBox()->clear();
    historyIndex_ = std::size_t(-1);
    savedInput_.clear();
}

void MachGuiConsoleDropDown::doDisplay()
{
    const Gui::Box box = absoluteBoundary();

    GuiPainter::instance().filledRectangle(box, MachGui::CONSOLEFILLCOLOR());

    const int separatorY = std::max(box.minCorner().y() + borderThickness_, box.maxCorner().y() - inputAreaHeight_);
    GuiPainter::instance().line(
        Ren::Point(box.left() + borderThickness_ / 2, separatorY),
        Ren::Point(box.right() - borderThickness_ / 2, separatorY),
        MachGui::CONSOLEBORDERCOLOR(),
        borderThickness_);

    const Gui::Coord origin = box.minCorner();

    if (pConsole_)
    {
        GuiPainter::instance().hollowRectangle(box, MachGui::CONSOLEBORDERCOLOR(), borderThickness_);
        GuiPainter::instance().blit(promptBmp_, Gui::Coord(origin.x() + promptCoord_.x(), origin.y() + promptCoord_.y()));
    }

    if (consoleOutput_.has_value())
    {
        GuiPainter::instance().blit(consoleOutput_.value(), Gui::Coord(origin.x() + contentCoord_.x(), origin.y() + contentCoord_.y()));
    }
}

void MachGuiConsoleDropDown::focusInput()
{
    GuiManager::instance().charFocus(inputBox_.get());
    inputBox_->setCursorPosition(inputBox_->text().size());
}

void MachGuiConsoleDropDown::blurInput()
{
    if (GuiManager::instance().charFocusExists() && &GuiManager::instance().charFocus() == inputBox_.get())
    {
        GuiManager::instance().removeCharFocus();
    }
}

void MachGuiConsoleDropDown::updateInput()
{
    if (isVisible())
    {
        inputBox_->update();
    }
}

MachGuiSingleLineEditBox* MachGuiConsoleDropDown::inputBox() const
{
    return inputBox_.get();
}

void MachGuiConsoleDropDown::setOpen(bool open)
{
    open_ = open;
}

bool MachGuiConsoleDropDown::isOpen() const
{
    return open_;
}

void MachGuiConsoleDropDown::toggle()
{
    setOpen(!open_);
}

void MachGuiConsoleDropDown::close()
{
    setOpen(false);
}

bool MachGuiConsoleDropDown::doHandleKeyEvent(const GuiKeyEvent& event)
{
    if (event.state() != Gui::PRESSED)
        return true;

    if (event.key() == Device::KeyCode::TAB)
    {
        handleTabCompletion();
    }
    else
    {
        lastTabWasComplete_ = false;

        if (event.isCtrlPressed() && event.key() == Device::KeyCode::KEY_V)
        {
            handlePaste();
        }
        else if (event.isCtrlPressed() && event.key() == Device::KeyCode::KEY_L)
        {
            if (pConsole_ != nullptr)
                pConsole_->clearOutput();
        }
        else if (event.key() == Device::KeyCode::ENTER)
        {
            submit();
        }
        else if (event.key() == Device::KeyCode::ESCAPE)
        {
            toggle();
        }
        else if (event.key() == Device::KeyCode::UP_ARROW)
        {
            navigateHistory(-1);
        }
        else if (event.key() == Device::KeyCode::DOWN_ARROW)
        {
            navigateHistory(1);
        }
        else
        {
            inputBox()->doHandleKeyEvent(event);
        }
    }

    return true;
}

std::string MachGuiConsoleDropDown::inputText() const
{
    return inputBox()->text();
}

void MachGuiConsoleDropDown::setInputText(const std::string& text)
{
    inputBox()->setText(text);
    inputBox()->setCursorPosition(text.size());
}

void MachGuiConsoleDropDown::navigateHistory(int direction)
{
    if (!pConsole_)
        return;

    const auto& history = pConsole_->history();
    if (history.empty())
        return;

    const std::size_t count = history.size();
    const std::size_t npos = std::size_t(-1);

    if (direction < 0)
    {
        // UP — go to older entry
        if (historyIndex_ == npos)
        {
            // Start browsing: save current input, go to newest history entry
            savedInput_ = inputText();
            historyIndex_ = count - 1;
        }
        else if (historyIndex_ > 0)
        {
            --historyIndex_;
        }
        else
        {
            return;
        }
        setInputText(history[historyIndex_]);
    }
    else
    {
        // DOWN — go to newer entry
        if (historyIndex_ == npos)
            return;

        if (historyIndex_ + 1 < count)
        {
            ++historyIndex_;
            setInputText(history[historyIndex_]);
        }
        else
        {
            // Past newest entry: restore saved input
            historyIndex_ = npos;
            setInputText(savedInput_);
            savedInput_.clear();
        }
    }
}

void MachGuiConsoleDropDown::handleTabCompletion()
{
    if (!pConsole_)
        return;

    const std::string line = inputText();
    const std::size_t cursor = inputBox()->cursorPosition();
    const System::IConsole::CompletionResult result = pConsole_->suggestions(line, cursor);
    const std::vector<std::string>& candidates = result.candidates;

    if (candidates.empty())
        return;

    if (lastTabWasComplete_)
    {
        // Second consecutive Tab: echo the prompt+input, then list alternatives.
        std::string echoLine{pConsole_->prompt()};
        echoLine += line;
        pConsole_->writeLine(echoLine);

        for (const std::string& candidate : candidates)
        {
            pConsole_->writeLine(candidate);
        }
        if (result.totalCount > candidates.size())
        {
            pConsole_->writeLine("and " + std::to_string(result.totalCount - candidates.size()) + " more");
        }
        lastTabWasComplete_ = false;
        return;
    }

    // Helper: replace the token region in the line and set cursor after the replacement.
    auto applyReplacement = [&](const std::string& replacement)
    {
        std::string newLine = line.substr(0, result.replaceStart)
            + replacement
            + line.substr(result.replaceStart + result.replaceLength);
        const std::size_t newCursor = result.replaceStart + replacement.size();

        inputBox()->setText(newLine);
        inputBox()->setCursorPosition(newCursor);
    };

    if (candidates.size() == 1)
    {
        std::string replacement = candidates[0];

        // Append a space only when:
        // - there is no text after the replaced region, or
        // - the character right after the replaced region is not already a space.
        const std::size_t afterToken = result.replaceStart + result.replaceLength;
        if (afterToken >= line.size()
            || !std::isspace(static_cast<unsigned char>(line[afterToken])))
        {
            replacement += ' ';
        }

        applyReplacement(replacement);
        lastTabWasComplete_ = false;
        return;
    }

    // Multiple matches: complete to the longest common prefix of candidates.
    const std::string lcp = Utils::longestCommonPrefix(candidates);

    const std::string_view currentToken = std::string_view(line).substr(
        result.replaceStart, result.replaceLength);
    if (lcp.size() > currentToken.size())
    {
        applyReplacement(lcp);
    }

    lastTabWasComplete_ = true;
}

void MachGuiConsoleDropDown::handlePaste()
{
    const std::string clipboard = GuiClipboard::instance().getText();
    if (clipboard.empty())
        return;

    constexpr std::size_t maxLines = 16;
    constexpr std::size_t maxCharsPerLine = 255;

    // Split into lines, handling \r\n, \r, and \n
    std::vector<std::string> lines;
    std::string current;
    for (std::size_t i = 0; i < clipboard.size() && lines.size() < maxLines; ++i)
    {
        const char c = clipboard[i];
        if (c == '\r')
        {
            // \r\n counts as one line break
            if (i + 1 < clipboard.size() && clipboard[i + 1] == '\n')
                ++i;
            lines.push_back(std::move(current));
            current.clear();
        }
        else if (c == '\n')
        {
            lines.push_back(std::move(current));
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    // If the clipboard text didn't end with a newline, keep the remainder as a partial line
    // (it won't be submitted automatically)
    if (lines.size() >= maxLines)
    {
        current.clear();
    }

    // First line: append to whatever is already in the input box
    if (!lines.empty())
    {
        std::string firstLine = inputText() + lines[0];
        if (firstLine.size() > maxCharsPerLine)
            firstLine.resize(maxCharsPerLine);
        setInputText(firstLine);
        submit();

        // Remaining complete lines
        for (std::size_t i = 1; i < lines.size(); ++i)
        {
            std::string line = lines[i];
            if (line.size() > maxCharsPerLine)
                line.resize(maxCharsPerLine);
            setInputText(line);
            submit();
        }
    }

    // Insert any trailing text (no newline at end) into the input box
    if (!current.empty())
    {
        std::string partial = inputText() + current;
        if (partial.size() > maxCharsPerLine)
            partial.resize(maxCharsPerLine);
        setInputText(partial);
    }
}

void MachGuiConsoleDropDown::doLayout()
{
    const int width = static_cast<int>(GuiDisplayable::width());
    const int height = static_cast<int>(GuiDisplayable::height());
    const int separatorY = std::max(borderThickness_, height - inputAreaHeight_);
    const int areaBottom = std::max(borderThickness_, height - borderThickness_);
    const int minX = std::max(borderThickness_ + inputPadding_, 0);
    const int maxX = std::max(minX + 1, width - borderThickness_ - inputPadding_);
    const int bestInputY = separatorY + borderThickness_ + inputPadding_ / 2 + 1 * Gui::uiScaleFactor();
    const int minY = std::min(areaBottom - inputPadding_, bestInputY);
    const int maxY = std::min(areaBottom, minY + static_cast<int>(inputFont_.height()));

    contentCoord_ = Gui::Coord(minX, borderThickness_ + 1 * Gui::uiScaleFactor());
    promptCoord_ = Gui::Coord(minX, minY);
    inputBox_->setRelativeBoundary(Gui::Box(minX + promptBmp_.width(), minY, maxX, maxY));
}

void MachGuiConsoleDropDown::prerenderOutput()
{
    if (width() <= 0 || height() <= 0)
    {
        consoleOutput_.reset();
        return;
    }

    const int horizontalPadding = contentCoord_.x();
    const int outputWidth = std::max(1, static_cast<int>(width()) - horizontalPadding * 2);
    const int outputHeight = std::max(1, static_cast<int>(height()) - inputAreaHeight_ - borderThickness_);

    consoleOutput_ = RenSurface::createAnonymousSurface(Ren::Size(outputWidth, outputHeight));

    Ren::Painter painter(consoleOutput_.value());
    painter.clearRectangle(Ren::Rect(0, 0, outputWidth, outputHeight));

    Ren::Point drawPos { 0, 0 };
    const int lineHeight = static_cast<int>(inputFont_.height()) + 1 * Gui::uiScaleFactor();
    const int maxVisibleLines = consoleOutput_->height() / lineHeight;
    const std::size_t totalLines = outputLines_.size();
    const std::size_t linesToRender = std::min<std::size_t>(maxVisibleLines, totalLines);
    const std::size_t startIndex = totalLines - linesToRender;
    for (std::size_t i = startIndex; i < totalLines; ++i)
    {
        if (drawPos.y + static_cast<int>(inputFont_.height()) > outputHeight)
        {
            break;
        }

        painter.drawText(outputLines_[i], drawPos, inputFont_, consoleOutput_->width());
        drawPos.y += lineHeight;
    }
}

void MachGuiConsoleDropDown::setOutput(const std::vector<std::string>& lines)
{
    outputLines_ = lines;
    prerenderOutput();
}

void MachGuiConsoleDropDown::updateOutputFromConsole()
{
    if (pConsole_ == nullptr)
    {
        outputLines_.clear();
        prerenderOutput();
        return;
    }

    setOutput(pConsole_->output());
}

void MachGuiConsoleDropDown::updatePromptSurface(std::string_view promptText)
{
    if (promptText.empty())
    {
        promptBmp_ = {};
        return;
    }

    const int promptWidth = std::max(1, inputFont_.horizontalAdvance(promptText));
    promptBmp_ = RenSurface::createAnonymousSurface(Ren::Size(promptWidth, inputFont_.height()));
    Ren::Painter painter(promptBmp_);
    painter.drawText(promptText, {}, inputFont_, promptBmp_.width());
}
