/***********************************************************

 A L S A M P L E . H P P

***********************************************************/

/*

  ALSample

  By using DXSound this completes a simple DirectSound
  encapsulation library.

  Above the interface that IDirectSoundBuffer provides
  the construction code makes use of the utilty classes
  Waveform and WaveFormat. So done to allow more
  flexibility on maintenance with streaming being a
  simple-ish addition because of Waveforms read method.

*/

#ifndef A_L_SAMPLE_HPP
#define A_L_SAMPLE_HPP

#include "sound/internal/sample.hpp"

class DevTimer;

///////////////////////////////////////////////////////////

class ALSample : public Sample
{
public:
    enum SampleState
    {
        INITIALISED = 0,
        PLAYING = 1,
        STOPPED = 2,
        PLAYING_OUT = 4,
        STOPPING = 5
    };

    enum LoopStatus
    {
        SINGLE_LOOP,
        INFINITE_LOOP
    };

    enum LockType
    {
        FULL_LOCK,
        FIRST_HALF_LOCK,
        SECOND_HALF_LOCK
    };

    enum MemoryType
    {
        UNKNOWN,
        HARDWARE,
        SOFTWARE
    };

    ALSample(const SndSampleParameters& params);

    ~ALSample() override;

    void play() override;
    void stop() override;
    void stopAtEnd() override;
    bool isPlaying() override;
    Snd::RelativeTime length() const override;
    void volume(int newVolume) override;
    Sample::SampleState update() override;
    void pause() override;
    void restart() override;
    void restart(int offset) override;

    void silence() override;
    void unsilence() override;

    void newPosition(const MexVec3& newPos) override;
    void position3D(MexVec3* newPos) override;

protected:
    int lockAndLoad(int from, LockType lockType);
    bool isValid() const;

    ALuint alBuffer_, alSource_;
    SndWaveform* waveform_;

    size_t currentWaveFilePosition_;

    size_t bufferSize_;
    size_t id_;
    size_t loopCount_;
    SampleState state_;
    int bufferIncrement_;
    SampleStorage storageType_;

    Snd::RelativeTime startOffset_;

    const SndSampleParameters sampleParameters_;

    DevTimer* internalTimer_;

    bool stopAtEnd_;

    // Is this sample using the DX notification points?
    // This will be false if the wave is static, and infinetly
    // playing or only playing once.
    bool usingNotificationForStop_;

    // Where does this sample live? Hardware or software
    MemoryType sampleResidence_;

    friend class SndMixer;
};

////////////////////////////////////////////////////////////
#ifdef _INLINE
#include "alsample.ipp"
#endif

////////////////////////////////////////////////////////////

#endif /* A_L_SAMPLE_HPP **********************************/
