# Texture Management Refactoring

## Current Design Assessment

### The Good

- **Ref-counted sharing via the manager** — `RenSurfaceManager` acts as a flyweight factory: `createTexture("foo.bmp")` returns the same `RenITexBody` if already loaded, with ref counting. This avoids duplicate loads.
- **Separation of handle (`RenSurface`) from body (`RenISurfBody`)** — clients hold lightweight ID-based handles, the manager owns the actual data.

### The Problems

**1. `RenSurface` conflates two unrelated concepts**

`RenSurface` is simultaneously:
- A **2D drawing canvas** (blit, fill, line, text, pixel get/set, save PNG)
- A **texture handle** used for 3D rendering

These are fundamentally different use cases. The back buffer is a drawing target you blit GUI onto. A texture is an immutable image bound to a material for 3D meshes. Lumping them together means every texture carries the full 2D drawing API, and `RenTexture` inherits from `RenSurface` just to add `hasAlphaTransparency()` and `usesBilinear()`.

**2. `RenISurfBody` is a god class**

It handles: file I/O, SDL surface conversion, GPU texture allocation, colour key emulation, alpha map merging, mipmap generation, blit operations, text rendering, ref counting, name management, sharing policy, and display buffer identity. That's ~850 lines doing 10+ distinct jobs.

**3. The `castToTexBody()` virtual is a code smell**

`RenISurfBody` has `virtual RenITexBody* castToTexBody()` — manual RTTI. The manager stores both surfaces and textures in the same `ctl_vector<RenISurfBody*>` and uses `castToTexBody()` to distinguish them. This is a sign the two types shouldn't share a base class.

**4. The ID-based indirection is fragile**

`RenSurface` holds a `Ren::TexId` (integer index into `entries_[]`). Every operation goes through `internals()` → manager lookup → dereference. This adds overhead and makes the code harder to follow. The original motivation was DirectDraw surface management; with modern GL where textures are just opaque handles, this indirection layer adds complexity without benefit.

**5. `Residence` enum is vestigial**

`TEXTURE`, `SYSTEM`, `VIDEO` — these mapped to DirectDraw surface types. Now `allocateDDSurfaces()` does the same thing regardless of residence: creates a GL texture. The distinction is meaningless.

**6. Dead/vestigial methods**

- `restoreToVRAM()`, `loadIntoVRAM()`, `recreateVRAMSurface()` — all return `true` unconditionally
- `releaseDC()`, `setDDColourKey()`, `updateDescr()` — DirectDraw leftovers
- `extractBMPText()` — opens a file and returns `true` without reading anything
- The `createAnonymousSurface` overloads still take a `const RenSurface&` parameter (formerly for pixel format matching) that is now unused

## Proposed Design

| Class | Responsibility |
|-------|---------------|
| **`RenTexture`** | Immutable GPU texture handle. Created by the texture manager. Ref-counted. Has name, alpha flag, bilinear flag, `BackendTextureHandle`. |
| **`RenTextureManager`** | Flyweight factory for textures. Owns the name→texture map, search paths, file loading. |
| **`RenSurface`** | 2D drawing canvas for GUI/HUD. Wraps a `BackendTextureHandle` as a render target. Has blit, fill, line, text, pixel ops. Not related to textures. |
| **`RenISurfBody`** | Removed or reduced to a thin render-target wrapper. File I/O moves to the texture manager. Blit/draw ops move to `RenSurface`. |

The key insight is: **textures and drawing surfaces share almost no behavior**. The only thing they have in common is "wraps a GPU texture handle," which is a one-liner, not a reason for inheritance.

## Practical Path Forward

A full rewrite is risky given how many callers exist. A pragmatic incremental approach:

1. **Remove dead methods** (`restoreToVRAM`, `loadIntoVRAM`, `recreateVRAMSurface`, `releaseDC`, `setDDColourKey`, `extractBMPText`, `Residence` enum)
2. **Remove the unused `const RenSurface&` parameter** from `createAnonymousSurface`/`createAnonymousVideoSurface`
3. **Clean up D3D comments** still in `surfbody.hpp` and `surfmgr.cpp`
4. **Longer term**: split `RenISurfBody` into a texture body and a surface body, eliminate `castToTexBody()`
