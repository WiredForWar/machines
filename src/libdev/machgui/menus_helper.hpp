#pragma once

#include "gui/gui.hpp"

namespace machgui {
namespace helper {
namespace menus {

    // Get an x value starting from the screen's left. x:[0, WIDTH] - Optionally divide if getting the top-left of a centered image
    int x_from_screen_left(int distance, int divisor = 1);
    // Get a y value starting from the screen's bottom. y:[0, HEIGHT] - Optionally divide if getting the top-left of a centered image
    int y_from_screen_bottom(int distance, int divisor = 1);

    // Set the origin of `box` to the top-left of a `width` by `height` bitmap that's centered on the screen.
    Gui::Box centered_bitmap_transform(const Gui::Box& box, int width, int height);

}
}
} // end machgui::helper::menus
