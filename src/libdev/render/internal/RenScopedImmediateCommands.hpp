#pragma once

class RenDevice;

// RAII guard that ensures a command buffer is active for recording.
// If neither the frame command buffer nor an immediate command buffer
// is currently active, this guard begins an immediate command buffer
// on construction and ends (submits) it on destruction.
// If any command buffer is already active, this is a no-op.
//
// Place at the top of rendering entry points (blit, fill, draw, etc.)
// so that callers outside the frame loop don't need to manually manage
// command buffers.
class RenScopedImmediateCommands final
{
public:
    explicit RenScopedImmediateCommands(RenDevice* device);
    ~RenScopedImmediateCommands();

    RenScopedImmediateCommands(const RenScopedImmediateCommands&) = delete;
    RenScopedImmediateCommands& operator=(const RenScopedImmediateCommands&) = delete;

private:
    RenDevice* device_{};
    bool started_{};
};
