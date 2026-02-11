#include "render/internal/RenScopedImmediateCommands.hpp"

#include "render/device.hpp"

RenScopedImmediateCommands::RenScopedImmediateCommands(RenDevice* device)
    : device_(device)
{
    if (!device->rendering() && !device->immediateCommandsActive())
    {
        device_->beginImmediateCommands();
        started_ = true;
    }
}

RenScopedImmediateCommands::~RenScopedImmediateCommands()
{
    if (started_)
        device_->endImmediateCommands();
}
