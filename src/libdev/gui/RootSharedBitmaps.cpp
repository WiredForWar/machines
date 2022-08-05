#include "RootSharedBitmaps.hpp"

GuiRootSharedBitmaps::GuiRootSharedBitmaps()
    : GuiRootSharedBitmaps(GuiPainter::instance())
{
}

GuiRootSharedBitmaps::GuiRootSharedBitmaps(const IGuiPainter& painter)
    : guiPainter_(painter)
{
}

GuiRootSharedBitmaps::~GuiRootSharedBitmaps()
{
}
