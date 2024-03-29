#include "machines/sdlapp.hpp"

#include "base/diag.hpp"

#include "machgui/ctxoptns.hpp"
#include "sound/soundmix.hpp"
#include "sound/sndparam.hpp"
#include "world4d/soundman.hpp"
#include "recorder/recorder.hpp"
#include "device/cd.hpp"
#include "system/registry.hpp"

#include "spdlog/spdlog.h"

void debugTiming(const char*, bool);

void SDLApp::initSound()
{
    spdlog::info("Initializing sound...");

    int nMaxSamples = 20;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        nMaxSamples = 30;

    SndMixerParameters soundParams(
        //     window(), // Where this is the HWND of your application window
        Snd::Polyphony(5), // The Mixers polyphony
        SndMixerParameters::DIRECTSOUND, // You must be using DIRECTSOUND
        Snd::ELEVEN_THOUSAND_HZ, // The playback sample rate
        Snd::SIXTEEN_BIT, // The playback "bit-ness" ?!
        Snd::STEREO, // Speaker setup
        Snd::THREE_D, // THREE_D or TWO_D
        nMaxSamples // Number of samples that can exist at one time, an assertion will occur if this limit is
                    // overstepped.
    );

    SndMixer::initialise(soundParams);

    savedSampleVolume_ = SndMixer::instance().masterSampleVolume();
    savedCDVolume_ = DevCD::instance().volume();

    spdlog::info("Sample volume: {}", savedSampleVolume_);
    spdlog::info("Music volume: {}", savedCDVolume_);

    W4dSoundManager::instance();
}

void SDLApp::loadSounds()
{
    spdlog::info("Loading sounds...");

    const bool enable3d
        = SysRegistry::instance().queryIntegerValue("Options\\3DSound", "on", SndMixer::instance().is3dMixer());
    MachGuiCtxOptions::load3dSoundFiles(enable3d);
}

void SDLApp::cleanUpSound()
{
    W4dSoundManager::instance().unloadAll();

    SndMixer::instance().masterSampleVolume(savedSampleVolume_);
    DevCD::instance().volume(savedCDVolume_);
    SOUND_STREAM("Reset volumes" << std::endl);
}

void SDLApp::updateSound(const MexPoint3d& pos)
{
    // Update sound volumes every 5 frames
    debugTiming("SDLApp:: call SoundManger\n", true);

    static int volumeCount = 0;
    if (volumeCount == 0)
        W4dSoundManager::instance().updateVolumes(pos);
    volumeCount = (++volumeCount) % 5;

    // Update sounds playing
    W4dSoundManager::instance().updateCycle();
    debugTiming("SDLApp:: call SoundManger complete\n", false);
}
