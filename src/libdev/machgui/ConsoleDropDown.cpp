#include "ConsoleDropDown.hpp"

#include "gui/manager.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"
#include "render/Painter.hpp"

#include "system/IConsole.hpp"

#include <algorithm>
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

    prerenderOutput();
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

void MachGuiConsoleDropDown::toggle()
{
    open_ = !open_;
}

bool MachGuiConsoleDropDown::isOpen() const
{
    return open_;
}

bool MachGuiConsoleDropDown::doHandleKeyEvent(const GuiKeyEvent& event)
{
    if (event.state() != Gui::PRESSED)
        return true;

    if (event.key() == Device::KeyCode::ENTER)
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
