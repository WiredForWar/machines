#include "machgui/menus_helper.hpp"
#include "render/display.hpp"
#include "render/device.hpp"

int machgui::helper::menus::x_from_screen_left(int distance, int divisor)
{
    divisor = (divisor == 0) ? 1 : divisor;
    const RenDisplay::Mode& mode = RenDevice::current()->display()->currentMode();
    int p = std::max(0, mode.width() - distance);

    return p / divisor;
}

int machgui::helper::menus::y_from_screen_bottom(int distance, int divisor)
{
    divisor = (divisor == 0) ? 1 : divisor;
    const RenDisplay::Mode& mode = RenDevice::current()->display()->currentMode();
    int p = std::max(0, mode.height() - distance);

    return p / divisor;
}

Gui::Box machgui::helper::menus::centered_bitmap_transform(const Gui::Box& box, int width, int height)
{
    return Gui::Box(Gui::Coord(box.minCorner().x() - x_from_screen_left(width, 2),
                               box.minCorner().y() - y_from_screen_bottom(height, 2)),
                    box.maxCorner().x() - box.minCorner().x(),
                    box.maxCorner().y() - box.minCorner().y()
    );
}
