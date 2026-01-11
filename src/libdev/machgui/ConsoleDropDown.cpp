#include "ConsoleDropDown.hpp"

#include "gui/event.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"

MachGuiConsoleDropDown::MachGuiConsoleDropDown(GuiDisplayable* parent) :
    GuiDisplayable(parent, GuiDisplayable::LAYER5)
{
    redrawEveryFrame(true);

    borderThickness_ = static_cast<int>(4 * Gui::uiScaleFactor());
}

void MachGuiConsoleDropDown::setViewportSize(Gui::Size size)
{
    size.setHeight(size.height() / 3);
    setRelativeBoundary(Gui::Box(0, 0, size.width(), size.height()));
}

void MachGuiConsoleDropDown::doDisplay()
{
    const Gui::Box box = absoluteBoundary();

    GuiPainter::instance().filledRectangle(box, MachGui::CONSOLEFILLCOLOR());
    GuiPainter::instance().hollowRectangle(box, MachGui::CONSOLEBORDERCOLOR(), borderThickness_);
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
    if (event.key() == Device::KeyCode::ESCAPE)
    {
        toggle();
    }

    return true;
}
