#include "render/Pixels.hpp"

#include <SDL3_image/SDL_image.h>

#include <cstddef>

namespace Ren
{

namespace PixelsImpl
{
namespace
{

// Wraps the rows in an SDL surface without copying them, so they have to outlive
// the result.
SDL_Surface* asSdlSurface(const Pixels& pixels)
{
    // SDL_PIXELFORMAT_RGBA32 selects the byte-order-correct format on both
    // little and big endian systems.
    return SDL_CreateSurfaceFrom(
        pixels.width,
        pixels.height,
        SDL_PIXELFORMAT_RGBA32,
        const_cast<unsigned char*>(pixels.rgba.data()),
        pixels.width * 4);
}

} // namespace
} // namespace PixelsImpl

std::vector<unsigned char> encodePng(const Pixels& pixels)
{
    if (pixels.isEmpty())
        return {};

    SDL_Surface* surface = PixelsImpl::asSdlSurface(pixels);
    if (! surface)
        return {};

    std::vector<unsigned char> png;

    SDL_IOStream* stream = SDL_IOFromDynamicMem();
    if (stream && IMG_SavePNG_IO(surface, stream, false))
    {
        const Sint64 size = SDL_GetIOSize(stream);
        if (size > 0 && SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET) == 0)
        {
            png.resize(static_cast<std::size_t>(size));
            if (SDL_ReadIO(stream, png.data(), png.size()) != png.size())
                png.clear();
        }
    }

    if (stream)
        SDL_CloseIO(stream);

    SDL_DestroySurface(surface);

    return png;
}

} // namespace Ren
