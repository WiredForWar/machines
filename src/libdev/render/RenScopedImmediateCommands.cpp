#include "render/internal/RenScopedImmediateCommands.hpp"

#include "render/Device.hpp"

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
