#pragma once

#include "render/render.hpp"

#include <string_view>

class RenIRenderBackend
{
public:
    virtual ~RenIRenderBackend() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool isInitialized() const = 0;
};
