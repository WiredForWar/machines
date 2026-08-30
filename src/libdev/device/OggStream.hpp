#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include <cstdint>

#include "al.h"

struct stb_vorbis;

// Streams an Ogg Vorbis file onto a caller-owned OpenAL source using a
// 3-buffer queue refilled from a background thread. End-of-stream is reported
// through a callback fired from that thread.
class OggStream
{
public:
    using FinishedCallback = std::function<void()>;

    // 'source' is owned by the caller and MUST outlive this object; call stop()
    // (or destroy this object) before deleting the source. 'onFinished' is
    // invoked once when playback finishes; it is not invoked by stop().
    OggStream(ALuint source, FinishedCallback onFinished);
    ~OggStream();

    OggStream(const OggStream&) = delete;
    OggStream& operator=(const OggStream&) = delete;

    // Opens 'filePath', with playback starting 'startSeconds' into the track.
    // A start past the end of the track falls back to the beginning. Returns
    // false (leaving the object unplayable) on error.
    bool open(const std::string& filePath, double startSeconds = 0.0);

    // Primes the buffer queue, starts the source and the refill thread.
    void play();

    // Stops the refill thread and the source and unqueues all buffers.
    // Safe to call more than once; never invokes the finished callback.
    void stop();

    // The position the listener is hearing, in seconds from the start of the
    // track -- not the (later) position the decoder has read to. Meaningful
    // from play() until stop().
    double playheadSeconds() const;

private:
    void refillLoop();
    // Decodes one buffer's worth of PCM into 'buffer' and uploads it. Returns
    // false when the decoder has reached end-of-file with no more samples.
    bool fillBuffer(ALuint buffer);

    static constexpr int NumBuffers = 3;

    ALuint source_;
    FinishedCallback onFinished_;

    stb_vorbis* vorbis_{};
    int channels_{};
    int sampleRate_{};
    ALenum format_{};
    int bufferShorts_{}; // interleaved shorts per buffer (0.25 s of audio)

    // Which buffer is which for the playhead bookkeeping below.
    int bufferIndex(ALuint buffer) const;

    ALuint buffers_[NumBuffers]{};

    std::int64_t startSample_{}; // where in the track decoding started
    mutable std::mutex playheadMutex_;
    std::int64_t playedSamples_{}; // per channel, in buffers already unqueued
    int bufferSamples_[NumBuffers]{}; // per channel, per buffer while queued

    std::thread thread_;
    std::atomic<bool> running_{};
    bool eof_{};
};
