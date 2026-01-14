#pragma once

#include <memory>

namespace Utils
{

struct CallbackHandle
{
    virtual ~CallbackHandle() = default;
};

struct HandleWithTrigger : public CallbackHandle
{
    virtual void trigger() const = 0;
};

using CallbackHandleUPtr = std::unique_ptr<Utils::CallbackHandle>;
using HandleWithTriggerUPtr = std::unique_ptr<Utils::HandleWithTrigger>;

} // namespace Utils
