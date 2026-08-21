#pragma once

#include "base/base.hpp"
#include "render/BackendType.hpp"
#include "render/ShaderSet.hpp"
#include "render/Texture.hpp"
#include "render/internal/BackendCommands.hpp"
#include "render/internal/DrawCallFactory.hpp"
#include "render/internal/TriangleGroup.hpp"
#include "render/render.hpp"

#include "render/PrimitiveTopology.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "utility/CallbackHandle.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

class SysPathName;

class MexQuad3d;
class MexPoint2d;
class MexPoint3d;

class RenDisplay;
class RenColour;
class RenCamera;
class RenLight;
class RenMaterial;
class RenStats;
class RenSurface;
class RenISurfBody;

class RenIDeviceImpl;
class RenIVertexData;

namespace Ren
{
class IRenderBackend;

// What RenDevice::addResourcesInvalidatedCallback registers into. Named out here
// only because the handle it hands back has to reach the same list to take
// itself off again; nothing else has business with it.
struct ResourcesInvalidatedRegistry
{
    struct Entry
    {
        const Utils::CallbackHandle* handle{};
        std::function<void()> callback{};
    };

    std::vector<Entry> entries{};
};
}

// A device object represents a rendering target.  It orchestrates
// the rendering process.

class RenDevice final
{
public:
    // Uses the display to create front and back buffers; possibly changes
    // the display resolution; and generally sets everything up for 3D
    // rendering. Calls useDevice(this).
    // PRE(Ren::initialised());
    // PRE(MexCoordSystem::instance().isSet());
    RenDevice(RenDisplay*);
    ~RenDevice();

    // One-time initialization: creates the render backend, sets up the display,
    // allocates all GPU resources (pipelines, buffers, shadow maps, etc.).
    // Must be called exactly once before any rendering.
    bool initialize(Ren::BackendType backendType = Ren::BackendType::Auto);

    // We can fail to start the rendering in which case false is returned.  If
    // clearBack is false, then the back buffer isn't cleared (but z is).
    // Calls useDevice(this).
    // PRE(!rendering());
    // POST(implies(result, idleRendering()));
    bool startFrame();

    // Sets the rendering parameters appropriately for 2D gui stuff.
    // PRE(idleRendering());
    // POST(rendering2D());
    void start2D();
    // PRE(rendering2D());
    void end2D();

    // Sets up the 3D rendering context (matrices, stats, illuminator).
    // Shadow passes may be issued after this call and before beginGeometryPass().
    // PRE(idleRendering());
    // POST(rendering3D());
    void start3D();
    // Opens the main geometry render pass (binds FBO, clears, sets fog/depth/blend).
    // PRE(rendering3D());
    void beginGeometryPass(bool clearBack = true);
    // PRE(rendering3D());
    void end3D();

    // PRE(rendering());
    // PRE(!doingBackground_);
    void startBackground(double yon);

    // PRE(idleRendering());
    // POST(!rendering());
    // POST(!rendering3D() and !rendering2D());
    // POST(!doingBackground_);
    void endFrame();

    // Finalize the back buffer: draw debug text overlay and cursor.
    void finalizeBackBuffer();

    // Present the back buffer to the screen (swap buffers).
    // PRE(rendering());
    // POST(!rendering());
    void presentFrame();

    // True within calls of startFrame() and endFrame().
    bool rendering() const;
    // True within calls of start2D() and end2D().
    bool rendering2D() const;
    // True within calls of start3D() and end3D().
    bool rendering3D() const;
    // True when rendering() is true and when rendering2D() and rendering3D() are false.
    bool idleRendering() const;

    // Immediately clear all display surfaces to a given flat colour.  If no
    // colour is specified, the current background colour is used.  This is
    // useful for getting everything into a known state before starting to
    // compose a scene.  The entire screen area is cleared.  If a cursor is
    // (or was) displayed any saved areas are discarded.
    void clearAllSurfaces(const RenColour&); // PRE(!rendering());
    void clearAllSurfaces(); // PRE(!rendering());

    bool switchBackend(Ren::BackendType type);

    // Which GLSL dialect the scene is drawn with, and which ones the backend in
    // use is able to compile. Choosing between them changes where sources are
    // read from and nothing else, so it is a way to compare two writings of the
    // same shaders rather than a visual setting.
    Ren::ShaderSet shaderSet() const;
    std::vector<Ren::ShaderSet> supportedShaderSets() const;

    // Build the pipelines again from the given set. Returns false, and leaves
    // the current set in place, if the backend cannot compile it.
    bool setShaderSet(Ren::ShaderSet set); // PRE(!rendering());

    // Register a callback invoked after GPU resources are invalidated and
    // reloaded (e.g. backend switch, scale factor change).  Listeners
    // should release cached anonymous surfaces and rebuild their context.
    //
    // Registration lasts as long as the returned handle. Keep it for as long as
    // whatever the callback reaches into: a callback usually closes over the
    // object that owns the surface it redraws, and one left registered past that
    // object is a call through a dangling pointer.
    [[nodiscard]] Utils::CallbackHandleUPtr addResourcesInvalidatedCallback(std::function<void()> callback);
    void fireResourcesInvalidatedCallbacks();

    void reset();
    virtual void setMaterialHandles(const RenMaterial& mat);

    void initializeDisplay();

    // This sets the sub-area of the screen which is used for 3D rendering.  The
    // remainder of the back-buffer is accessible thru the RenSurface interface.
    // The device only clears the area inside this rectangle, any remaining parts
    // of the buffer (borders, gui, etc.) are the client's responsibility.
    // Defaults to full-screen if you never call this method.
    // PRE(!rendering3D());
    void setViewport(int left, int top, int width, int height);

    // Set the viewport to the given size and clear the colour buffer to black.
    // Used by the display layer after a mode change.
    void clearDisplay(int width, int height);

    // Use another camera.  At present, multiple viewports on one device
    // aren't supported, so you can't switch cameras mid-frame.
    // PRE(cam);
    // PRE(!rendering3D());
    // PRE(cam->hitherClipDistance() < cam->yonClipDistance());
    void useCamera(RenCamera* cam);

    // Which camera is currently in use?
    RenCamera* currentCamera() const;

    // Return the coordinates of screenPosition (defined in screen 2d pixel space)
    // in the 3d world coordinate frame of the camera
    MexPoint3d screenToCamera(const MexPoint2d& screenPosition) const;

    // Return the coordinates of worldPosition (defined in in the 3d world coordinate frame of the camera)
    // in screen 2d pixel space
    MexPoint2d cameraToScreen(const MexPoint3d& worldPosition) const;

    // Modify the list of objects which can potentially light the scene.
    // PRE(light);
    void addLight(RenLight* light);
    void removeLight(RenLight* light);

    // The default value is a white light of 0.3 intensity.
    const RenColour& ambient() const;
    void ambient(const RenColour&);

    // This has the effect of turning all lights off.  Polygons will be drawn with
    // ambient and self-luminous lighting only.  If a client knows that a mesh is
    // all black or self-luminous, it will be drawn much more efficiently if
    // lighting is diabled.
    void disableLighting();
    void enableLighting();
    bool lightingEnabled() const;

    // Although there can be more than one render device in existence,
    // we should never need two simultaneously.  The client is expected to
    // set a current device before using any other rendering functionality.
    // N.B. a device *must* be set before loading meshes or textures.
    static void useDevice(RenDevice* d);
    static RenDevice* current();

    void recordCommand(Ren::BackendCommand&& command);
    void recordEnableVertexAttribPointer(
        Ren::AttributeLocationId index,
        int size,
        Ren::BackendVertexAttribType type,
        bool normalized,
        std::size_t stride,
        std::size_t offset);
    void recordDisableVertexAttribPointer(Ren::AttributeLocationId index);

    void enableVertexLayout(
        Ren::AttributeLocationId posAttr,
        int posComponents,
        Ren::AttributeLocationId uvAttr,
        Ren::AttributeLocationId colAttr);
    void disableVertexLayout(
        Ren::AttributeLocationId posAttr,
        Ren::AttributeLocationId uvAttr,
        Ren::AttributeLocationId colAttr);

    void beginImmediateCommands();
    void endImmediateCommands();
    bool immediateCommandsActive() const;

    // Submit all pending frame commands so that subsequent GPU readbacks
    // see up-to-date results.  A new command buffer is started for the
    // remainder of the frame.  No-op if no frame buffer is recording.
    void flushCommandBuffer();

    void backgroundColour(const RenColour&);
    const RenColour& backgroundColour() const;

    // Densities of around 0.05 work.
    // PRE(start > 0); PRE(start < end); PRE(density >= 0 && density <= 1);
    void fogOn(float start, float end, float density);
    void fogOn();
    void fogOff();
    const RenColour& fogColour() const;
    void fogColour(const RenColour& newFogColour);
    void fogEnd(float end);

    // These effect temporary changes to the fog parameters.  The status quo
    // can be restored by calling restoreFog.  Primarily for W4dBackground.
    // If restoreFog isn't called, endFrame restores the parameters anyway.
    void disableFog();
    void overrideFog(float start, float end, float density);
    void overrideFog(float start, float end, float density, const RenColour&);
    void restoreFog();

    void setVSyncPreference(bool enabled);
    bool vsyncEnabled() const { return vsyncEnabled_; }

    // Fog query methods.
    bool isFogOn() const;
    float fogStart() const;
    float fogEnd() const;
    float fogDensity() const;

    // The dimensions of the display, window or whatever.
    Ren::Size windowSize() const;
    int windowWidth() const;
    int windowHeight() const;

    // Point visibility test
    // PRE(currentCamera());
    bool canSee(const MexPoint3d& pt) const;
    // Quad visibility test which supports domain/portal culling.
    // PRE(currentCamera());
    bool canSee(const MexQuad3d& quad) const;

    // Quad visibility test which supports domain/portal culling.
    // Uses the given camera instead of currentCamera().
    // PRE(currentCamera());
    bool canSee(const RenCamera* cam, const MexQuad3d& quad) const;

    // Display an image on this device's surface -- typically this would be
    // used for displaying a "please wait" screen.
    void displayImage(const SysPathName& pathName);

    // Interference covers a lot of the screen and looks like a badly tunned TV.
    // High interference values will entirely obliterate the image.  On the other
    // hand, static (see below) hardly obscures anything.  Interference carries a
    // fairly high performance penalty in terms of screen over-writes.
    // Calling interferenceOff is equivalent to interferenceOn(0) just more wordy.
    void interferenceOn(double amount); // PRE(amount >=0 && amount <= 1);
    void interferenceOff();
    double interferenceAmount() const;
    bool isInterferenceOn() const;

    // Adds a random speckles to the frame buffer.  TBD: perhaps this should
    // belong in RenCamera, like colourFilter?
    void staticOn();
    void staticOff();
    bool isStaticOn() const;

    // The display on which this device is rendered.
    RenDisplay* display();
    const RenDisplay* display() const;

    bool setHighestAllowedDisplayMode();

    // Create surfaces which provide access to the front and back buffers.
    RenSurface backSurface();
    RenSurface frontSurface();
    const RenSurface backSurface() const;
    const RenSurface frontSurface() const;

    // Enable edge anti-aliasing.  In the only currently available implementation
    // (nVidia TNT), this is *very* slow.  It should probably only be turned on
    // for screenshots.  There will be no effect if the h/w doesn't support it.
    void antiAliasingOn(bool);
    bool antiAliasingOn() const;

    void setSmoothScaleEnabled(bool enabled);

    // Clients can print any debugging text here.  Any text will be overlayed
    // on the 3D graphics.  The 3D is drawn first, then any internal text such
    // as frame rate, then the client's text below that.
    std::ostream& out();

    // The debug text appears at these co-ordinates.  (Well actually, the internal
    // stuff like stats appears first, then the debug text below that).
    void debugTextCoords(int x, int y);
    void debugTextCoords(int* x, int* y) const;

    friend std::ostream& operator<<(std::ostream& o, const RenDevice& t);

    bool activate();

    // Statistics describing the performance of the render system.  The return
    // value can be null, indicating that statistics are not being gathered.
    RenStats* statistics();
    const RenStats* statistics() const;

    // colour filter

    class Filter
    {
    public:
        Filter(RenDevice* pDevice, const RenColour& col); // call setFilter
        ~Filter(); // call resetFilter

    private:
        RenDevice* const pDevice_;

        Filter();
        Filter(const Filter&);
        const Filter& operator=(const Filter&);
    };

    RenIDeviceImpl& impl();
    const RenIDeviceImpl& impl() const;

    Ren::IRenderBackend& backend();
    const Ren::IRenderBackend& backend() const;

    void renderScreenspace(
        const RenIVertex* vertices,
        const size_t nVertices,
        Ren::PrimitiveTopology topology,
        const int targetW = 0,
        const int targetH = 0)
    {
        renderScreenspace(vertices, nVertices, topology, targetW, targetH, Ren::NullTexId);
    };

    void renderScreenspace(
        const RenIVertex* vertices,
        const size_t nVertices,
        const RenMaterial& mat,
        Ren::PrimitiveTopology topology,
        const int targetW = 0,
        const int targetH = 0);
    void renderSurface(
        const RenISurfBody* surf,
        const Ren::Rect& srcArea,
        const Ren::Rect& dstArea,
        const uint32_t targetW = 0,
        const uint32_t targetH = 0,
        const uint32_t colour = 0xFFFFFFFF,
        Ren::BlitMode mode = Ren::BlitMode::AlphaBlend);
    void renderPrimitive(
        const RenIVertex* vertices,
        const size_t nVertices,
        const RenMaterial& mat,
        Ren::PrimitiveTopology topology = Ren::PrimitiveTopology::TriangleFan);
    void renderIndexed(
        const RenIVertex* vertices,
        const size_t nVertices,
        const Ren::VertexIdx* indices,
        const size_t nIndices,
        const RenMaterial& mat,
        Ren::PrimitiveTopology topology);
    void renderIndexedScreenspace(
        const RenIVertex* vertices,
        const size_t nVertices,
        const Ren::VertexIdx* indices,
        const size_t nIndices,
        const RenMaterial& mat,
        Ren::PrimitiveTopology topology);

    void setModelMatrix(glm::mat4& model) { model_ = model; }

    const glm::mat4& getModelMatrix() { return model_; }

    const glm::mat4& getProjectionMatrix() { return projection_; }

    const glm::mat4& getViewMatrix() { return view_; }

    // Where the scene is viewed from, supplied by the caller rather than
    // derived from the current camera and the display mode.
    //
    // A caller that decides its own framing needs this: covering an image in
    // several passes, or filling a viewport whose frustum is not centred on its
    // axis, neither of which a camera described by one vertical field of view
    // and the window's aspect ratio can express. Nothing below the device knows
    // a viewpoint was supplied, so the same scene can be drawn more than once
    // per frame from different ones.
    struct Viewpoint
    {
        glm::mat4 view{};
        glm::mat4 projection{};
    };

    // Render subsequent frames from here. Takes effect at the next start3D().
    void setViewpoint(const Viewpoint& viewpoint);

    // Go back to deriving both matrices from the current camera.
    void clearViewpoint();

    bool hasViewpoint() const;

    // Shadow mapping: render scene depth from the light's perspective.
    enum class ShadowCascade { Near, Far };
    void beginShadowPass(ShadowCascade cascade, const glm::mat4& lightSpaceMatrix);
    void endShadowPass();
    void setShadowSplitDistance(float d);
    bool isShadowPassActive() const;
    bool isShadowMappingEnabled() const;
    void shadowStrength(float s);
    float shadowStrength() const;
    void renderShadowDepth(
        const RenIVertex* vertices,
        size_t nVertices,
        const Ren::VertexIdx* indices,
        size_t nIndices,
        Ren::PrimitiveTopology topology);
    void renderToTextureMode(Ren::TexId targetTexture, uint32_t viewPortW, uint32_t viewPortH);

    // private:
    void renderScreenspace(
        const RenIVertex* vertices,
        const size_t nVertices,
        Ren::PrimitiveTopology topology,
        const int targetW,
        const int targetH,
        Ren::TexId texture);

private:
    void createViewport();
    void updateMatrices();
    void displayStats();
    glm::mat4 cameraViewMatrix() const;
    glm::mat4 cameraProjectionMatrix(double hither, double yon) const;
    void setFog(float, float, float, const RenColour&);
    void addStatic();
    void addInterference();
    void graduatedNoisePolygon(const Ren::Rect& area, double minAlpha, double maxAlpha);
    void uniformNoisePolygon(const Ren::Rect& area, double maxAlpha);
    void blitPostProcess();
    void commonEndFrame();

    bool createGpuResources();
    void releaseGpuResources();
    void applyConfiguredShaderSet();

    bool setVSync(bool enabled);

    friend class Filter;
    void setFilter(const RenColour&);
    void resetFilter();

    // Sets the rendering parameters appropriately for the 3D world.
    // Draws all alpha polygons. start3D must have been called first.
    // PRE(rendering());
    // PRE(rendering3D());
    void flush3DAlpha();

    // Change the clipping planes mid-frame.  The effect lasts until endFrame
    // is called.  Intended for drawing backgrounds.
    // PRE(hither < yon);
    void overrideClipping(double hither, double yon);

    Ren::FrameState buildFrameState() const;
    Ren::GpuLightingState buildGpuLightingState(bool gpuLighting) const;
    Ren::StandardPipelineHandles buildStandardHandles() const;

    RenIDeviceImpl* pImpl_ { nullptr };

    glm::mat4 model_ {};
    glm::mat4 view_ {};
    glm::mat4 projection_ {};
    std::optional<Viewpoint> viewpoint_ {};
    glm::vec3 fogColour_ {};
    glm::vec3 fogParams_ {};

    bool clearAll2D_ { false };
    bool vsyncEnabled_ { true };

    bool standardUniformsDirty_{};
    bool billboardUniformsDirty_{};

    // Held by shared pointer so that a handle outliving the device unregisters
    // from nothing instead of from freed memory.
    std::shared_ptr<Ren::ResourcesInvalidatedRegistry> resourcesInvalidated_{
        std::make_shared<Ren::ResourcesInvalidatedRegistry>()
    };

    // Operations deliberately revoked
    RenDevice(const RenDevice&);
    RenDevice& operator=(const RenDevice&);
    bool operator==(const RenDevice&);
};
