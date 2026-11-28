#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3) return 2;
    int maximumDimension = 2048; // Original HUD asset budget remains the default.
    if (argc == 3)
    {
        try { maximumDimension = std::stoi(argv[2]); }
        catch (...) { return 2; }
        if (maximumDimension < 1 || maximumDimension > 8192) return 2;
    }
    const std::filesystem::path root(argv[1]);
    std::ifstream manifest(root / "native-check.tsv");
    if (!manifest) return 3;
    unsigned count = 0;
    std::uint64_t decodedBytes = 0;
    std::string line;
    while (std::getline(manifest, line))
    {
        std::istringstream row(line);
        std::string relative;
        int expectedWidth = 0, expectedHeight = 0;
        std::getline(row, relative, '\t');
        row >> expectedWidth >> expectedHeight;
        const auto path = (root / relative).string();
        SDL_Surface* loaded = IMG_Load(path.c_str());
        if (!loaded)
        {
            std::cerr << relative << ": " << SDL_GetError() << '\n';
            return 4;
        }
        if (loaded->w != expectedWidth || loaded->h != expectedHeight
            || loaded->w <= 0 || loaded->h <= 0
            || loaded->w > maximumDimension || loaded->h > maximumDimension)
        {
            std::cerr << "Invalid texture dimensions: " << relative << '\n';
            return 5;
        }
        // Match RenISurfBody::copyWithColourKeyEmulation without opening a
        // graphics device or uploading anything to the user's GPU.
        SDL_Surface* converted = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA8888);
        SDL_Surface* destination = SDL_CreateSurface(loaded->w, loaded->h, SDL_PIXELFORMAT_RGBA32);
        if (!converted || !destination
            || !SDL_SetSurfaceColorKey(converted, true, SDL_MapSurfaceRGB(converted, 255, 0, 255))
            || !SDL_BlitSurface(converted, nullptr, destination, nullptr))
        {
            std::cerr << "Engine-style conversion failed: " << relative << ": " << SDL_GetError() << '\n';
            return 6;
        }
        decodedBytes += std::uint64_t(loaded->w) * loaded->h * 4;
        ++count;
        SDL_DestroySurface(destination);
        SDL_DestroySurface(converted);
        SDL_DestroySurface(loaded);
    }
    std::cout << "Decoded and converted " << count << " PNG textures with SDL3_image; "
              << decodedBytes << " RGBA bytes across all scale variants. No video window opened.\n";
    return count > 0 ? 0 : 7;
}
