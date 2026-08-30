#include "device/OggStream.hpp"

#include "spdlog/spdlog.h"

#include <chrono>
#include <utility>
#include <vector>

// Only the declarations are needed here; the implementation is built separately.
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

namespace
{
// 0.25 s of audio per buffer, refilled every 0.125 s.
constexpr float BufferSeconds = 0.25f;
constexpr auto RefillInterval = std::chrono::milliseconds(125);
}

OggStream::OggStream(ALuint source, FinishedCallback onFinished)
    : source_(source)
    , onFinished_(std::move(onFinished))
{
}

OggStream::~OggStream()
{
    stop();
    if (vorbis_ != nullptr)
    {
        stb_vorbis_close(vorbis_);
        vorbis_ = nullptr;
    }
}

bool OggStream::open(const std::string& filePath, double startSeconds)
{
    int error = 0;
    vorbis_ = stb_vorbis_open_filename(filePath.c_str(), &error, nullptr);
    if (vorbis_ == nullptr)
    {
        spdlog::warn("OggStream: could not open '{}' (stb_vorbis error {})", filePath, error);
        return false;
    }

    const stb_vorbis_info info = stb_vorbis_get_info(vorbis_);
    channels_ = info.channels;
    sampleRate_ = static_cast<int>(info.sample_rate);
    format_ = (channels_ == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    bufferShorts_ = static_cast<int>(sampleRate_ * BufferSeconds) * channels_;

    if (startSeconds > 0.0)
    {
        const unsigned int startSample = static_cast<unsigned int>(startSeconds * sampleRate_);
        if (stb_vorbis_seek(vorbis_, startSample) != 0)
        {
            startSample_ = startSample;
        }
        else
        {
            stb_vorbis_seek_start(vorbis_);
        }
    }

    return true;
}

int OggStream::bufferIndex(ALuint buffer) const
{
    for (int i = 0; i < NumBuffers; ++i)
    {
        if (buffers_[i] == buffer)
        {
            return i;
        }
    }
    return 0;
}

double OggStream::playheadSeconds() const
{
    if (sampleRate_ == 0)
    {
        return 0.0;
    }

    // The lock keeps the source's offset consistent with playedSamples_: the
    // refill thread unqueues buffers (which rebases the offset) under it too.
    const std::lock_guard<std::mutex> lock(playheadMutex_);
    ALint sampleOffset = 0;
    alGetSourcei(source_, AL_SAMPLE_OFFSET, &sampleOffset);
    return static_cast<double>(startSample_ + playedSamples_ + sampleOffset) / sampleRate_;
}

bool OggStream::fillBuffer(ALuint buffer)
{
    std::vector<short> pcm(bufferShorts_);
    // Returns the number of samples PER CHANNEL actually decoded, 0 at EOF.
    const int samplesPerChannel
        = stb_vorbis_get_samples_short_interleaved(vorbis_, channels_, pcm.data(), bufferShorts_);
    if (samplesPerChannel <= 0)
    {
        return false;
    }

    const ALsizei bytes = static_cast<ALsizei>(samplesPerChannel * channels_ * sizeof(short));
    alBufferData(buffer, format_, pcm.data(), bytes, sampleRate_);
    {
        const std::lock_guard<std::mutex> lock(playheadMutex_);
        bufferSamples_[bufferIndex(buffer)] = samplesPerChannel;
    }
    return true;
}

void OggStream::play()
{
    alGenBuffers(NumBuffers, buffers_);

    int queued = 0;
    for (int i = 0; i < NumBuffers; ++i)
    {
        if (!fillBuffer(buffers_[i]))
        {
            eof_ = true;
            break;
        }
        alSourceQueueBuffers(source_, 1, &buffers_[i]);
        ++queued;
    }

    if (queued == 0)
    {
        // Nothing to play: report completion as at a normal end-of-track.
        if (onFinished_)
        {
            onFinished_();
        }
        return;
    }

    alSourcePlay(source_);

    running_ = true;
    thread_ = std::thread(&OggStream::refillLoop, this);
}

void OggStream::refillLoop()
{
    while (running_)
    {
        ALint processed = 0;
        alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);

        while (processed-- > 0)
        {
            ALuint buffer = 0;
            {
                const std::lock_guard<std::mutex> lock(playheadMutex_);
                alSourceUnqueueBuffers(source_, 1, &buffer);
                playedSamples_ += bufferSamples_[bufferIndex(buffer)];
            }

            if (!eof_)
            {
                if (fillBuffer(buffer))
                {
                    alSourceQueueBuffers(source_, 1, &buffer);
                }
                else
                {
                    eof_ = true;
                }
            }
        }

        ALint queued = 0;
        alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);

        if (queued == 0)
        {
            // All decoded audio has drained: end of track.
            if (running_ && onFinished_)
            {
                onFinished_();
            }
            running_ = false;
            break;
        }

        // Recover from an underrun: the source stopped with buffers still queued.
        ALint state = 0;
        alGetSourcei(source_, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING)
        {
            alSourcePlay(source_);
        }

        std::this_thread::sleep_for(RefillInterval);
    }
}

void OggStream::stop()
{
    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }

    // The caller guarantees the source is still valid here (stop() runs before
    // the source is deleted).
    alSourceStop(source_);

    ALint queued = 0;
    alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0)
    {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(source_, 1, &buffer);
    }

    if (buffers_[0] != 0)
    {
        alDeleteBuffers(NumBuffers, buffers_);
        buffers_[0] = buffers_[1] = buffers_[2] = 0;
    }
}
