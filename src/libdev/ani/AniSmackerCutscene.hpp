#pragma once

#include <cstddef>
#include "ani/AniSmackerRegular.hpp"

class FrameScaler;

class AniSmackerCutscene : public AniSmackerRegular
{
public:
    AniSmackerCutscene(const SysPathName& path, size_t xCoordTo, size_t yCoordTo, size_t fsWidth, size_t fsHeight);
    ~AniSmackerCutscene() override;

    AniSmackerCutscene(const AniSmackerCutscene& other) = delete;
    AniSmackerCutscene& operator=(const AniSmackerCutscene) = delete;
    bool operator==(const AniSmackerCutscene&) const = delete;

protected:
    RenSurface createSmackerSurface(RenDevice* pDevice) override;
    void copyCurrentVideoFrameToBuffer(RenSurface& renderSurface) override;

private:
    FrameScaler* fetchOrInitializeScaler();

    FrameScaler* pFrameScaler_;
    size_t fullScreenWidth_;
    size_t fullScreenHeight_;
    size_t scaledVideoHeight_;
};
