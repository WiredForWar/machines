# Renderer Modernization Plan

## Design Principles

- **Legacy first.** The game must run on OpenGL 2.1 hardware. The original look must
  remain available as a config option (`LightingMode::Legacy`).
- **Every expensive feature is optional.** Whenever we add something costly (shadow
  mapping, SSAO, bloom, etc.) it must be gated behind a config variable so users can
  tune for their hardware.
- **Four backends, eventually.** OpenGL 2.1 (current), OpenGL 3.3+ core (desktop),
  OpenGL ES 3.x (mobile & web via Emscripten/WebGL 2), Vulkan (desktop, ray-tracing).
  GL 3.3 and ES 3.x share most code via a common modern-GL base. We finish what we
  can on GL 2.1 first, then port to GL 3.3+/ES, then Vulkan.
- **No new art required.** All visual improvements must work with the existing assets.
  Procedural techniques (auto-generated normal maps, SSAO, bloom, colour grading) are
  preferred. Artist-authored content is a nice-to-have for the future.

---

## Current State (what is already done)

### Backend Abstraction (complete)
- `Ren::IRenderBackend` interface with `RenderBackendGL` implementation.
- Command buffer system (`BackendCommand` variant, record/submit).
- Pipeline abstraction (`PipelineDesc`, logical shader names, cached uniform/attrib
  locations in `StandardPipelineLocations`, `Gui2DPipelineLocations`,
  `BillboardPipelineLocations`).
- Render pass abstraction (`RenderPassDesc`, `LoadOp`/`StoreOp`).
- Uniform buffer objects: CPU-side structs (`Gui2DUniforms`, `StandardFrameUniforms`,
  `StandardObjectUniforms`, `BillboardUniforms`, `ShadowDepthUniforms`,
  `PostProcessUniforms`) replace individual `recordSetUniform*` calls.
- Vertex input descriptions: `bindPipeline` auto-enables/disables attrib arrays via
  bitset diff against state cache.
- No direct `gl*` calls or `<GL/glew.h>` includes outside `src/libdev/render/OpenGL/`.
  All buffer/FBO variable names are backend-agnostic.
- `IRenderBackend::create()` factory — `devicei.cpp` does not know the concrete type.
- Deferred texture deletion, per-texture filter in `bindTexture2D` command.
- `readPixelsFloat` / `readPixelsUByte` backend methods (no `glReadPixels` outside
  backend).

### Shaders
- Four pipelines: `StandardShading`, `2DShading`, `BillboardShading`, `ShadowDepth`.
- Two shader directories: `data/shaders/120/` (GLSL 1.20) and `data/shaders/`
  (GLSL 310 ES, currently unused at runtime — the GL 2.1 backend loads from `120/`).

### GPU Lighting (complete on GL 2.1)
- `LightingMode` config: `Legacy` (CPU per-vertex) and `PerPixel` (GPU fragment shader).
- Per-vertex GPU path: normals expanded from indexed fixed-point to flat float[3] VBO,
  Lambertian computed in vertex shader, falls back to CPU for meshes with per-vertex
  materials.
- Per-pixel GPU path: `fragWorldPos` passed from vertex shader, fragment shader
  computes Lambertian + point light loop with attenuation.
- Up to 16 GPU point lights. Both `RenIPointLight` and `RenIUniformLight` collected
  via `RenIAttenuatedLight` base. Per-light `omni` flag distinguishes them in shader.
- In PerPixel mode, LOCAL lights are promoted to DYNAMIC (domain-based assignment) so
  weapon flashes, healing beams, etc. illuminate everything nearby.

### 2-Cascade Shadow Mapping (complete on GL 2.1)
- `ShadowQuality` config: `None`, `Hard`, `Soft`.
- Near cascade: 4096×4096, dynamic extent based on camera height above terrain.
- Far cascade: 2048×2048, larger extent for distant shadows.
- Split distance: view-space distance for cascade selection in fragment shader.
- PCF 3×3 soft shadows with slope-scaled bias and edge-fade for steep surfaces.
- Shadow pass: no face culling + `glPolygonOffset` for terrain compatibility.
- `doNotLight` entities skipped during shadow depth pass; `noShadowCast` flag for
  entities that should not cast shadows (e.g. holographs).
- `isShadow` entities skipped during main render when GPU shadows active.
- Cascade center computed from camera position + forward vector, terrain-aware height
  via callback (`setTerrainHeightFunction`).

### Post-Processing Pipeline (complete on GL 2.1)
- Render to intermediate colour+depth FBO.
- `PostProcess` shader pair for tone mapping.
- `postProcessFBO_`, `postProcessColorTexture_`, `postProcessQuadVBO_` resources.
- Full-screen blit pass after geometry + UI.

---

## Phase 1 — Finish GL 2.1 Visual Improvements

Everything here must work on the current OpenGL 2.1 backend with GLSL 1.20 shaders.
Each item is independently toggleable via config.

### ~~1.1 Specular Highlights (Blinn-Phong)~~ — done
### ~~1.2 Directional Light Shadow Mapping~~ — done (2-cascade CSM)
### ~~1.3 Soft Shadows (PCF)~~ — done (3×3 PCF with edge-fade)

### 1.4 Auto-Generated Normal Maps
- At texture load time, generate a normal map from the diffuse texture using a
  Sobel filter (greyscale → height → normals). Store as a second texture.
- Compute tangent vectors from triangle edges at mesh load time (expand vertex data).
- Sample normal map in fragment shader, perturb the interpolated normal.
- Config: new `NormalMapping` toggle (on/off).
- Effort: **large** (vertex data expansion + tangent computation + shader work).
- Note: this is a "squeeze the assets" technique — no artist needed.

### 1.5 Ambient Occlusion (SSAO)
- Screen-space pass after the geometry pass: sample depth buffer, compute occlusion.
- Intermediate colour+depth FBO already exists (post-processing pipeline).
- Blur pass to smooth the AO term.
- Config: new `AmbientOcclusion` toggle (on/off).
- Effort: **medium** (FBO infrastructure is in place).

### 1.6 Bloom / Glow
- Threshold bright pixels, downsample, blur, composite back.
- Particularly useful for explosions, plasma projectiles, ion cannon.
- Uses the existing post-processing FBO.
- Config: new `Bloom` toggle (on/off).
- Effort: **medium**.

### 1.7 Fog Improvements
- Current fog is linear vertex fog. Move to per-pixel fog in the fragment shader
  (already partially there).
- Optionally add height-based exponential fog for atmosphere.
- Config: part of `LightingMode::PerPixel`.
- Effort: **small**.

### Known Workarounds (to be resolved in Phase 2)

- **Delayed group GPU lighting restore**: Coplanar and alpha-sorted material
  groups are rendered via delayed paths that store pre-lit vertex copies.
  The original code called `clearGpuLightingState()` before rendering these
  groups, which disabled GPU lighting and shadows for them (e.g. landing pads).
  Current workaround: delayed groups store a `const RenIVertexData*` to the
  original mesh vertices and re-call `lightVertices()` before rendering to
  restore expanded normals and GPU lighting uniforms. This re-runs the full
  `lightVertices()` including redundant `copyCoords` + `computeLambertian`.
  **Proper fix (Phase 2):** decouple GPU lighting uniform state (light
  direction, shadow maps, per-frame UBO) from per-mesh vertex expansion.
  With real UBOs, lighting uniforms are set once per frame, and the delayed
  render path only needs to re-bind the correct vertex normals — no need to
  re-run the entire illuminator.

### Phase 1 Remaining Order
```
1.7 Fog (small, quick win)
  │
1.5 SSAO ──► 1.6 Bloom
  │
1.4 Normal Maps (can be done in parallel)
```

---

## Phase 2 — Modern OpenGL Backends (GL 3.3 core + ES 3.x) ← **current stage**

**Goal**: two new `IRenderBackend` implementations sharing a common modern-GL base
class. One targets desktop GL 3.3 core, the other targets ES 3.0/3.1 for mobile and
web (Emscripten → WebGL 2).

**Preparation already done:**
- All raw GL calls and `<GL/glew.h>` includes are confined to `src/libdev/render/OpenGL/`.
- `IRenderBackend::create()` factory decouples construction from the concrete type.
- `Viewport` struct, `getViewport()`, `clearDisplay()` on the interface — no GL types
  leak outside the backend.
- Buffer/FBO variable names in `devicei.hpp` are backend-agnostic.
- GLSL 310 ES shaders already exist in `data/shaders/` (currently unused at runtime).

### 2.1 Class Hierarchy
```
IRenderBackend
├── RenderBackendGL21          (current, renamed from RenderBackendGL)
└── RenderBackendModernGL      (new, shared base for GL 3.3 + ES 3.x)
    ├── RenderBackendGL33      (desktop GL 3.3 core profile)
    └── RenderBackendGLES3     (ES 3.0 / 3.1, WebGL 2)
```
- `RenderBackendModernGL` holds all code common to both profiles: VAOs, real UBOs,
  FBO management, shader compilation, state cache.
- `RenderBackendGL33` adds desktop-only features: geometry shaders, `gl_ClipDistance`,
  compute shaders (GL 4.3+), wider texture format support.
- `RenderBackendGLES3` handles ES-specific constraints: no geometry shaders, restricted
  texture formats, `highp`/`mediump` precision qualifiers, EGL context setup.

### 2.2 Shader Directories
- `data/shaders/120/` — GLSL 1.20 (GL 2.1 backend, existing).
- `data/shaders/330/` — GLSL 3.30 core (GL 3.3 backend).
- `data/shaders/310es/` — GLSL 310 ES (ES 3.1 backend; the existing `data/shaders/`
  already contains 310 ES shaders — move or symlink them here).
- Backend resolves logical shader names to the correct directory based on GL version.
- Shared GLSL logic (lighting, shadow sampling) can live in include files and be
  `#include`d by both 330 and 310es shaders (supported via `ARB_shading_language_include`
  on desktop, or a simple preprocessor in the shader loader).

### 2.3 Backend Selection
- Detect GL version at startup. Prefer GL 3.3+ if available, fall back to 2.1.
  On ES-only platforms (mobile, Emscripten), use `RenderBackendGLES3`.
- Config override: `gfxBackend` = `auto` | `gl21` | `gl33` | `gles`.
- `IRenderBackend::create()` factory selects the implementation.

### 2.4 Common Modern-GL Features (shared base)
- VAO-based vertex layout (required in both core and ES profiles).
- Real UBOs for uniform blocks (GL 2.1 backend unpacks them into individual calls).
- Instanced rendering for repeated meshes (trees, debris).
- `glMapBufferRange` for efficient buffer updates.

### 2.5 GL 3.3 Desktop-Only Features
- Geometry shaders for grass/foliage billboards (if desired).
- Transform feedback or compute shaders for particle systems (GL 4.3+).
- Multi-render-target (MRT) for deferred shading (optional, see Phase 3).

### 2.6 ES 3.x / WebGL 2 Specifics
- No geometry or tessellation shaders — billboard expansion must stay on CPU or use
  instanced quads.
- No `glPolygonMode` — wireframe debug must use line-draw fallback.
- Precision qualifiers (`highp`, `mediump`) in shaders.
- On Emscripten: EGL context via SDL2, asset packaging via virtual filesystem,
  `ASYNCIFY` or `PROXY_TO_PTHREAD` for blocking loads.
- On native mobile (Android/iOS): EGL/EAGL context, touch input mapping.
- ES 3.1 adds compute shaders — gate compute features behind a capability check
  rather than backend type.

### 2.7 Capability Queries
- `IRenderBackend` gains a `capabilities()` method returning a struct with flags:
  `hasGeometryShaders`, `hasComputeShaders`, `hasClipDistance`,
  `maxTextureSize`, `maxColorAttachments`, etc.
- Higher-level code queries capabilities instead of checking backend type, so features
  degrade gracefully across GL 3.3 / ES 3.1 / ES 3.0.

---

## Phase 3 — Advanced Rendering Techniques (GL 3.3+)

These require the GL 3.3+ backend from Phase 2.

### 3.1 Deferred Shading (optional path)
- G-buffer pass: albedo, normals, depth, material properties.
- Lighting pass: full-screen quad, accumulate all lights.
- Enables many more lights without per-object cost.
- Config: `RenderPath` = `Forward` | `Deferred`.

### ~~3.2 Cascaded Shadow Maps (CSM)~~ — done (moved to Phase 1, 2-cascade)

### 3.3 Point Light Shadows
- Cube map shadow rendering for point lights.
- Very expensive — limit to 1–2 closest/brightest lights.
- Config: `PointLightShadows` toggle.

### 3.4 Screen-Space Reflections (SSR)
- Ray-march in screen space using the depth buffer.
- Useful for metallic surfaces, water.
- Config: `SSR` toggle.

### 3.5 Temporal Anti-Aliasing (TAA)
- Jittered projection + temporal accumulation.
- Reduces aliasing without MSAA cost.
- Config: `AntiAliasing` = `None` | `MSAA` | `TAA`.

### ~~3.6 HDR + Tone Mapping~~ — done (post-process pipeline in place on GL 2.1)

---

## Phase 4 — Vulkan Backend

**Goal**: Vulkan backend for maximum performance and ray-tracing support.

### 4.1 `RenderBackendVulkan` Implementation
- `VkDevice`, `VkQueue`, `VkCommandBuffer`, synchronisation primitives.
- Memory management via VMA (Vulkan Memory Allocator).
- Implement `Ren::IRenderBackend` interface.

### 4.2 SPIR-V Shader Pipeline
- Offline compilation: GLSL → SPIR-V via `glslangValidator` or `shaderc`.
- `data/shaders/spirv/` directory.
- Shader reflection for automatic descriptor set layout.

### 4.3 Pipeline State Objects
- Vulkan PSOs bake all render state (blend, depth, raster, shaders, vertex layout).
- Pipeline cache for fast startup.

### 4.4 Descriptor Sets & Resource Binding
- Descriptor set layouts for textures, UBOs, SSBOs.
- Bindless textures if supported (`VK_EXT_descriptor_indexing`).

### 4.5 Backend Selection
- Extend `gfxBackend` config: `auto` | `gl21` | `gl33` | `vulkan`.
- Vulkan preferred on desktop if available and driver is recent enough.

---

## Phase 5 — Ray-Tracing (Vulkan)

Requires `VK_KHR_ray_tracing_pipeline` or `VK_KHR_ray_query`.

### 5.1 Acceleration Structures
- Build BLAS per unique mesh.
- Build TLAS per frame from instance transforms.
- Incremental TLAS rebuild for moving objects.

### 5.2 RT Shadows
- Replace rasterised shadow maps with ray-traced shadows.
- Soft shadows via multiple rays or cone tracing.
- Hybrid: rasterise primary visibility, RT for shadow rays only.

### 5.3 RT Reflections
- Trace reflection rays for metallic/glossy surfaces.
- Denoise with temporal accumulation.

### 5.4 RT Ambient Occlusion
- Replace SSAO with ray-traced AO for ground-truth quality.
- Short rays from surface point in hemisphere.

### 5.5 Full Path Tracing (stretch goal)
- Replace rasterisation entirely for reference-quality rendering.
- Requires aggressive denoising (temporal + spatial, or ML-based).

---

## Phase 6 — Asset Improvements (long-term, needs artist)

These are nice-to-haves that become possible once the rendering pipeline supports them.
They require artist-authored content.

### 6.1 PBR Material Conversion
- Author metallic/roughness textures for existing models.
- Update shaders to use Cook-Torrance BRDF.

### 6.2 Hand-Painted Normal Maps
- Replace auto-generated normal maps with artist-authored ones for key models
  (machines, buildings).

### 6.3 Environment Maps
- Bake or capture cubemap probes for reflections and ambient lighting.

### 6.4 Higher-Resolution Textures
- Upscale or repaint key textures (terrain, buildings).

---

## Summary & Dependencies

```
Phase 1 (GL 2.1 improvements)
  ├── 1.1 Specular
  ├── 1.2–1.3 Shadow Mapping
  ├── 1.4 Normal Maps
  ├── 1.5–1.6 SSAO + Bloom (need intermediate FBO)
  └── 1.7 Fog

Phase 2 (GL 3.3 core + ES 3.x backends)
  ├── 2a GL 3.3 desktop backend
  ├── 2b ES 3.x / WebGL 2 backend (shares base with 2a)
  └── depends on: Phase 1 complete (shaders mature, patterns established)

Phase 3 (Advanced GL 3.3+ techniques)
  └── depends on: Phase 2a

Phase 4 (Vulkan backend)
  └── depends on: Phase 2 (shared shader logic, mature IRenderBackend interface)

Phase 5 (Ray-tracing)
  └── depends on: Phase 4

Phase 6 (Asset improvements)
  └── independent, can happen whenever an artist is available
```

## Config Variables (current and planned)

| Variable | Values | Phase |
|----------|--------|-------|
| `gfxLightingMode` | `Legacy`, `PerPixel` | done |
| `gfxShadowQuality` | `None`, `Hard`, `Soft` | done |
| `gfxNormalMapping` | `Off`, `On` | 1.4 |
| `gfxAmbientOcclusion` | `Off`, `On` | 1.5 |
| `gfxBloom` | `Off`, `On` | 1.6 |
| `gfxAntiAliasing` | `None`, `MSAA`, `TAA` | 3.5 |
| `gfxRenderPath` | `Forward`, `Deferred` | 3.1 |
| `gfxBackend` | `auto`, `gl21`, `gl33`, `gles`, `vulkan` | 2.3 / 4.5 |
| `gfxPointLightShadows` | `Off`, `On` | 3.3 |
| `gfxSSR` | `Off`, `On` | 3.4 |
