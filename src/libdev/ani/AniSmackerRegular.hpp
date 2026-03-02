#pragma once

#include "ani/AniSmacker.hpp"
#include "system/pathname.hpp"
#include "render/surface.hpp"

#include <vector>

#include "al.h"

using smk = struct smk_t*;

class AniSmackerRegular : public AniSmacker
{
public:
    AniSmackerRegular(const SysPathName& path, int xCoordTo = 0, int yCoordTo = 0, bool fast = false);
    ~AniSmackerRegular() override;

    AniSmackerRegular(const AniSmackerRegular& other) = delete;
    AniSmackerRegular& operator=(const AniSmackerRegular) = delete;
    bool operator==(const AniSmackerRegular&) const = delete;

    bool open() override;

    void setPosition(int x, int y);
    void setFast(bool value);
    void setScaleFactor(float scaleFactor) override;

    bool isFinished() const override;

    Ren::Size size() const override;

    void playNextFrame(RenDevice* pDevice) override;
    void displaySummaryInfo() const override;

    void rewind() override;

    // Ability to switch between front and back buffer blitting.
    // If using the front buffer the code assumes that you are not flipping the buffers.
    void useFrontBuffer(bool ufb) override;
    bool useFrontBuffer() const override;

protected:
    RenSurface createSmackerSurface(RenDevice* pDevice) override;
    void copyCurrentVideoFrameToBuffer(RenSurface& renderSurface) override;
    uint* fillBufferForCurrentFrame() override;

    int xCoordTo_{};
    int yCoordTo_{};
    float scaleFactor_{};

private:
    static constexpr int AudioBufferCount = 8;

    void feedAudioBuffers();
    void unpackBufferToSurface(RenSurface dst, const RenSurface& src);

    bool isFinishedNoRecord() const;

    // Smacker needs to avoid doing certain calls on the last frame
    bool isPenultimateFrame() const;

    void getNextFrame();

    // Member Data
    smk pSmack_{};
    uint* pBuffer_{};
    RenSurface surface_;
    unsigned frame_{};
    Ren::Size size_{};
    double frameTime_{};
    double lastFrameTime_{};
    bool advanceToNextFrame_{};
    bool fast_{};
    bool useFrontBuffer_{};
    SysPathName fileName_;
    bool finished_{};

    // Sound — separate smk instance for audio-only streaming
    smk pSmackAudio_{};
    bool audioFinished_{};
    ALsizei alFrequency_{};
    ALenum alFormat_{};
    ALuint alBuffers_[AudioBufferCount]{};
    ALuint alSource_{};
    std::vector<ALuint> freedBuffers_;
};
