#include "ani/AniSmackerCutscene.hpp"

#include "ani/FrameScaler.hpp"

AniSmackerCutscene::AniSmackerCutscene(const SysPathName& path, size_t xCoordTo, size_t yCoordTo, size_t fsWidth, size_t fsHeight)
    : AniSmackerRegular(path, xCoordTo, yCoordTo, false),
      pFrameScaler_(nullptr),
      scaledVideoHeight_(0L)
{
    fullScreenWidth_ = fsWidth;
    fullScreenHeight_ = fsHeight;
}

AniSmackerCutscene::~AniSmackerCutscene() {
    if (pFrameScaler_ != nullptr) {
        delete pFrameScaler_;
    }
}

RenSurface AniSmackerCutscene::createSmackerSurface(RenDevice *pDevice)
{
    // maintain aspect ratio
    scaledVideoHeight_ = fullScreenWidth_ * AniSmackerRegular::height() / AniSmackerRegular::width();
    yCoordTo_ = ( fullScreenHeight_ - scaledVideoHeight_ ) / 2L;

    return RenSurface::createAnonymousSurface( fullScreenWidth_, scaledVideoHeight_, pDevice->backSurface() );
}


void AniSmackerCutscene::copyCurrentVideoFrameToBuffer(RenSurface& renderSurface) {
    uint8_t* frame = reinterpret_cast<uint8_t*>(fillBufferForCurrentFrame());

    renderSurface.copyFromRGBABuffer(reinterpret_cast<uint*>(fetchOrInitializeScaler()->scaleFrame(frame)));
}

FrameScaler* AniSmackerCutscene::fetchOrInitializeScaler() {
    if (pFrameScaler_ == nullptr) {
        // By now, width() & height() will have the smacker file's w/h values
        pFrameScaler_ = new FrameScaler(this->width(), this->height(), fullScreenWidth_, scaledVideoHeight_);
    }

    return pFrameScaler_;
}
