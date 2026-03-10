

#define _INSTANTIATE_TEMPLATE_CLASSES

#include "ctl/List.hpp"
#include "ctl/Vector.hpp"
#include "device/Keyboard.hpp"
#include "gui/Font.hpp"
#include "gui/internal/FontCore.hpp"

class GuiDisplayable;

void Gui_ti1()
{
    static ctl_vector<GuiDisplayable*> dummy1;
    static ctl_list<GuiDisplayable*> dummy2;
    static ctl_list<DevKey::ScanCode> dummy3;
    static ctl_vector<GuiBmpFontCoreCharData> dummy10;
    static ctl_vector<GuiBmpFont> dummy11;
}

/* End **************************************************************/
