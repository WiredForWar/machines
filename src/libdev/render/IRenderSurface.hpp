#pragma once

namespace Ren
{

// The minimum a render backend needs from its host: a drawable of a known size
// that can be presented. Deliberately free of any windowing concepts, so that
// hosts which do not own a window of their own -- an editor render widget, an
// XR compositor -- can implement it.
class IRenderSurface
{
public:
    virtual ~IRenderSurface() = default;

    virtual int width() const = 0;
    virtual int height() const = 0;

    enum class VSyncMode
    {
        Off,
        On,
        Adaptive,
    };

    virtual bool setVSyncMode(VSyncMode mode) = 0;
    virtual void swapBuffers() = 0;
};

// Parameters for GL context creation. Each backend passes the version it needs.
struct GLContextParams
{
    int majorVersion{2};
    int minorVersion{1};
    bool coreProfile{};
};

// Implemented by hosts able to provide an OpenGL context. A GL backend asks for
// this by dynamic_cast; a future Vulkan backend will look for its own
// subinterface instead. Virtual inheritance so that a host implementing several
// of them still has a single IRenderSurface.
class IGLRenderSurface : public virtual IRenderSurface
{
public:
    virtual bool createGLContext(const GLContextParams& params) = 0;
    virtual void destroyGLContext() = 0;
};

} // namespace Ren
