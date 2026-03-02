#include "ani/AniSmackerRegular.hpp"

#include "render/display.hpp"
#include "render/Painter.hpp"
#include "recorder/recorder.hpp"
#include "recorder/private/recpriv.hpp"
#include "device/time.hpp"

#include "sound/internal/alsound.hpp"

#include "smacker.h"

//////////////////////////////////////////////////////////////////////

static inline ALenum toALformat(short channels, short samples)
{
    bool stereo = (channels > 1);

    switch (samples)
    {
        case 16:
            if (stereo)
                return AL_FORMAT_STEREO16;
            else
                return AL_FORMAT_MONO16;
        case 8:
            if (stereo)
                return AL_FORMAT_STEREO8;
            else
                return AL_FORMAT_MONO8;
        default:
            return -1;
    }
}

AniSmackerRegular::AniSmackerRegular(const SysPathName& path, int xCoordTo, int yCoordTo, bool fast)
{
    xCoordTo_ = xCoordTo;
    yCoordTo_ = yCoordTo;
    scaleFactor_ = 1;
    advanceToNextFrame_ = true;
    fast_ = fast;
    fileName_ = path;

    alFormat_ = -1;
}

// DTOR
AniSmackerRegular::~AniSmackerRegular()
{
    if (pBuffer_)
        delete[] pBuffer_;

    if (pSmack_)
        smk_close(pSmack_);

    if (pSmackAudio_)
        smk_close(pSmackAudio_);

    if (alSource_)
    {
        alDeleteSources(1, &alSource_);
        alDeleteBuffers(AudioBufferCount, alBuffers_);
    }
}

bool AniSmackerRegular::open()
{
    if (pSmack_)
    {
        // Already opened
        return false;
    }

    pSmack_ = smk_open_file(fileName_.pathname().c_str(), SMK_MODE_MEMORY);
    // In debug version, assert that SmackOpen has worked
    ASSERT(pSmack_ != nullptr, "SmackOpen failed");
    if (!pSmack_)
        return false;

    long unsigned int w, h;
    smk_info_video(pSmack_, &w, &h, nullptr);
    size_.width = w;
    size_.height = h;

    return true;
}

void AniSmackerRegular::setScaleFactor(float scaleFactor)
{
    scaleFactor_ = scaleFactor;
}

bool AniSmackerRegular::isFinished() const
{
    bool finished = false;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        finished = RecRecorderPrivate::instance().playbackAniSmackerFinished();
    else
    {
        finished = isFinishedNoRecord();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordAniSmackerFinished(finished);
    }

    return finished;
}

void AniSmackerRegular::setPosition(int x, int y)
{
    xCoordTo_ = x;
    yCoordTo_ = y;
}

void AniSmackerRegular::setFast(bool value)
{
    fast_ = value;
}

Ren::Size AniSmackerRegular::size() const
{
    return size_;
}

void AniSmackerRegular::playNextFrame(RenDevice* pDevice)
{
    // First time in? If so open the smack animation
    if (!pSmack_)
    {
        if (!open())
        {
            finished_ = true;
            return;
        }
    }

    if (surface_.isNull())
    {
        // Open a separate smk instance for audio-only streaming
        pSmackAudio_ = smk_open_file(fileName_.pathname().c_str(), SMK_MODE_MEMORY);
        if (pSmackAudio_)
        {
            unsigned char a_tracks, a_channels[7], a_bitdepth[7];
            unsigned long a_rate[7];
            smk_info_audio(pSmackAudio_, &a_tracks, a_channels, a_bitdepth, a_rate);
            if (a_tracks & (1 << 0))
            {
                alFormat_ = toALformat(a_channels[0], a_bitdepth[0]);
                alFrequency_ = a_rate[0];

                smk_enable_audio(pSmackAudio_, 0, 1);
                smk_enable_video(pSmackAudio_, 0);
                smk_first(pSmackAudio_);

                // Set up OpenAL source and queue buffers
                alGenSources(1, &alSource_);
                alTestError("gen source smacker");
                alSourcef(alSource_, AL_PITCH, 1);
                ALfloat fVol = SndMixer::instance().masterSampleVolume() / 100.0f;
                alSourcef(alSource_, AL_GAIN, fVol);
                alSource3f(alSource_, AL_VELOCITY, 0, 0, 0);
                alSourcei(alSource_, AL_SOURCE_RELATIVE, AL_TRUE);
                alSource3f(alSource_, AL_POSITION, 0.0f, 0.0f, 0.0f);
                alGenBuffers(AudioBufferCount, alBuffers_);
                alTestError("gen buffers smacker");

                // Pre-fill all queue buffers from the audio stream
                for (int i = 0; i < AudioBufferCount && !audioFinished_; ++i)
                {
                    const unsigned char* pAudioBuffer = smk_get_audio(pSmackAudio_, 0);
                    unsigned long audioSize = smk_get_audio_size(pSmackAudio_, 0);
                    if (audioSize > 0)
                    {
                        alBufferData(alBuffers_[i], alFormat_, pAudioBuffer, audioSize, alFrequency_);
                        alTestError("pre-fill buffer smacker");
                        alSourceQueueBuffers(alSource_, 1, &alBuffers_[i]);
                    }
                    else
                    {
                        freedBuffers_.push_back(alBuffers_[i]);
                    }

                    if (smk_next(pSmackAudio_) == SMK_DONE)
                        audioFinished_ = true;
                }
            }
            else
            {
                smk_close(pSmackAudio_);
                pSmackAudio_ = nullptr;
            }
        }

        // Set up video-only on the main instance
        smk_enable_video(pSmack_, 1);
        smk_first(pSmack_);

        smk_info_all(pSmack_, nullptr, nullptr, &frameTime_);
        frameTime_ *= 0.000001;
        surface_ = this->createSmackerSurface(pDevice);
        pBuffer_ = new uint[size_.width * size_.height];

        ASSERT(pBuffer_ != nullptr, "");

        if (alSource_)
        {
            alSourcePlay(alSource_);
            alTestError("start play source smacker");
        }
        lastFrameTime_ = DevTime::instance().time();
    } // FIRST FRAME

    bool shouldRender = true;

    // This method is called every frame after changes, check if frame time has passed
    double timeNow = DevTime::instance().time();
    advanceToNextFrame_ = (timeNow - lastFrameTime_ >= frameTime_);

    // Keep the audio stream ahead of playback
    feedAudioBuffers();

    // Copy next frame from smacker file to the buffer.
    if (advanceToNextFrame_ || fast_)
    {
        copyCurrentVideoFrameToBuffer(surface_);
        lastFrameTime_ = timeNow;
        shouldRender = true;
    }

    if (shouldRender || !useFrontBuffer())
    {
        // Render the animation to a surface ( usually the screen ).
        if (useFrontBuffer())
        {
            unpackBufferToSurface(pDevice->frontSurface(), surface_);
            pDevice->display()->flipBuffers();
        }
        else
            unpackBufferToSurface(pDevice->backSurface(), surface_);
    }

    if (fast_)
    {
        getNextFrame();
    }
    else
    {
        if (advanceToNextFrame_)
        {
            getNextFrame();
        }
    }
    ++frame_;
}

void AniSmackerRegular::displaySummaryInfo() const
{
    double usf;
    unsigned long f;
    smk_info_all(pSmack_, nullptr, &f, &usf);
    printf(
        "Opened file %s\nWidth: %d\nHeight: %d\nFrames: %lu\nFPS: %f\n",
        fileName_.pathname().c_str(),
        size_.width,
        size_.height,
        f,
        1000000.0 / usf);

    unsigned char a_t, a_c[7], a_d[7];
    unsigned long a_r[7];

    smk_info_audio(pSmack_, &a_t, a_c, a_d, a_r);
    int i;
    for (i = 0; i < 7; i++)
    {
        printf("Audio track %d: %u bits, %u channels, %luhz\n", i, a_d[i], a_c[i], a_r[i]);
    }
}

void AniSmackerRegular::rewind()
{
    if (pSmack_)
    {
        smk_seek_keyframe(pSmack_, 0);
        finished_ = false;

        if (alSource_)
            alSourceStop(alSource_);
    }
}

void AniSmackerRegular::useFrontBuffer(bool ufb)
{
    useFrontBuffer_ = ufb;
}

bool AniSmackerRegular::useFrontBuffer() const
{
    return useFrontBuffer_;
}

RenSurface AniSmackerRegular::createSmackerSurface(RenDevice* pDevice)
{
    return RenSurface::createAnonymousSurface(size());
}

uint* AniSmackerRegular::fillBufferForCurrentFrame()
{
    const unsigned char* pal = smk_get_palette(pSmack_);
    const unsigned char* image_data = smk_get_video(pSmack_);
    uint* img_buff = pBuffer_;

    // Prepare a RGBA colours palette, TODO: this can be called once after file is loaded
    uint col_palette[256];
    for (int i = 0; i < 256; i++)
    {
        uint colour = 0xFF00;
        colour |= pal[(i * 3) + 2];
        colour <<= 8;
        colour |= pal[(i * 3) + 1];
        colour <<= 8;
        colour |= pal[(i * 3) + 0];
        col_palette[i] = colour;
    }

    // Fill image buffer
    unsigned int pixel = 0;
    for (int i = 0; i < size_.height; ++i)
    {
        for (int j = 0; j < size_.width; ++j)
        {
            img_buff[pixel] = col_palette[image_data[i * size_.width + j]];
            ++pixel;
        }
    }

    return img_buff;
}

void AniSmackerRegular::copyCurrentVideoFrameToBuffer(RenSurface& renderSurface)
{
    renderSurface.copyFromRGBABuffer(fillBufferForCurrentFrame());
}


void AniSmackerRegular::unpackBufferToSurface(RenSurface dst, const RenSurface& src)
{
    bool doUnpack = true;

    //  We need this extra check when we're playing back a recording because
    //  we might be pretending to continue to play even though the flic has
    //  actually finished.
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        doUnpack = ! isFinishedNoRecord();

    if (doUnpack)
    {
        ASSERT(! isFinishedNoRecord(), "");

        Ren::Painter dstPainter(dst);
        if(scaleFactor_ == 1)
        {
            dstPainter.blit(src, src.size(), Ren::Point(xCoordTo_, yCoordTo_));
        }
        else
        {
            Ren::Rect destRect(src.size() * scaleFactor_);
            destRect.originX = xCoordTo_;
            destRect.originY = yCoordTo_;
            dstPainter.stretchBlit(src, src.size(), destRect);
        }
    }
}

bool AniSmackerRegular::isFinishedNoRecord() const
{
    return finished_;
}

bool AniSmackerRegular::isPenultimateFrame() const
{
    unsigned long frame, frame_count;
    smk_info_all(pSmack_, &frame, &frame_count, nullptr);
    return frame == frame_count - 1;
}

void AniSmackerRegular::feedAudioBuffers()
{
    if (!alSource_)
        return;

    // Reclaim finished buffers
    ALint processed;
    alGetSourcei(alSource_, AL_BUFFERS_PROCESSED, &processed);
    if (processed > 0)
    {
        ALuint buffs[AudioBufferCount]{};
        alSourceUnqueueBuffers(alSource_, processed, buffs);
        for (int i = 0; i < processed; ++i)
            freedBuffers_.push_back(buffs[i]);
    }

    // Fill free buffers by advancing the audio stream
    while (!freedBuffers_.empty() && !audioFinished_)
    {
        const unsigned char* pAudioBuffer = smk_get_audio(pSmackAudio_, 0);
        unsigned long audioSize = smk_get_audio_size(pSmackAudio_, 0);
        if (audioSize > 0)
        {
            ALuint buf = freedBuffers_.back();
            freedBuffers_.pop_back();
            alBufferData(buf, alFormat_, pAudioBuffer, audioSize, alFrequency_);
            alTestError("buffer data smacker");
            alSourceQueueBuffers(alSource_, 1, &buf);
        }

        if (smk_next(pSmackAudio_) == SMK_DONE)
            audioFinished_ = true;
    }

    // Restart playback if the source has starved
    ALint state;
    alGetSourcei(alSource_, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING)
    {
        alSourcePlay(alSource_);
        alTestError("play source smacker");
    }
}

void AniSmackerRegular::getNextFrame()
{
    if (! isPenultimateFrame())
    {
        smk_next(pSmack_);
    }
    else
    {
        smk_next(pSmack_);
        finished_ = true;
    }
}
