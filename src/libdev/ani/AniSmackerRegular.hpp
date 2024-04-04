#pragma once

#include "ani/AniSmacker.hpp"
#include "system/pathname.hpp"
#include "render/surface.hpp"

#include "al.h"

using smk = struct smk_t*;

class AniSmackerRegular : public AniSmacker
{
public:
    AniSmackerRegular(const SysPathName& path, size_t xCoordTo, size_t yCoordTo);
    AniSmackerRegular(const SysPathName& path, size_t xCoordTo, size_t yCoordTo, bool fast);
    ~AniSmackerRegular() override;

    AniSmackerRegular(const AniSmackerRegular& other) = delete;
    AniSmackerRegular& operator=(const AniSmackerRegular) = delete;
    bool operator==(const AniSmackerRegular&) const = delete;

    void setScaleFactor(float scaleFactor) override;
    bool isFinished() const override;

    unsigned int height() const override;
    unsigned int width() const override;

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

    size_t xCoordTo_;
    size_t yCoordTo_;
    float scaleFactor_{};

private:
    void copyCurrentFrameToBuffer(RenSurface& dst);
    void unpackBufferToSurface(RenSurface dst, const RenSurface& src);

    bool isFinishedNoRecord() const;

    // Smacker needs to avoid doing certain calls on the last frame
    bool isPenultimateFrame() const;

    void getNextFrame();

    // Member Data
    smk pSmack_;
    uint* pBuffer_;
    RenSurface surface_;
    unsigned frame_;
    size_t width_, height_;
    double frameTime_;
    double lastFrameTime_;
    bool advanceToNextFrame_;
    bool fast_;
    bool useFrontBuffer_;
    SysPathName fileName_;
    bool finished_;

    // Sound
    ALsizei alFrequency_;
    ALenum alFormat_;
    ALuint alBuffers_[8], alSource_;
    std::vector<ALuint> freedBuffers_;
};
