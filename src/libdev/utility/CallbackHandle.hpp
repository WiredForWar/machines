#pragma once

#include <memory>

namespace Utils
{

struct CallbackHandle
{
    virtual ~CallbackHandle() = default;
};

using CallbackHandleUPtr = std::unique_ptr<Utils::CallbackHandle>;

} // namespace Utils
