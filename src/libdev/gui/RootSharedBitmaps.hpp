#pragma once

#include "gui/painter.hpp"

class GuiRootSharedBitmaps final
{
public:
    GuiRootSharedBitmaps();
    explicit GuiRootSharedBitmaps(const IGuiPainter& painter);

    ~GuiRootSharedBitmaps();

private:
    const IGuiPainter& guiPainter_;
};
