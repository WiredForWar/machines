#include "ConsoleDropDown.hpp"

#include "gui/Event.hpp"
#include "gui/GuiPainter.hpp"
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
    if (event.key() == Device::KeyCode::ESCAPE)
    {
        toggle();
    }

    return true;
}
