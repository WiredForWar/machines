#pragma once

#include <vector>

namespace Ren
{

// An image held in main memory: four bytes per pixel in RGBA order, with the
// rows running from the top down, the way the gui counts them.
struct Pixels
{
    int width{};
    int height{};
    std::vector<unsigned char> rgba{};

    bool isEmpty() const { return rgba.empty(); }
};

// The image as the bytes of a PNG file, for a caller that wants to send or store
// it rather than write it out. Empty if the image holds no pixels or the encoding
// failed.
std::vector<unsigned char> encodePng(const Pixels& pixels);

} // namespace Ren
