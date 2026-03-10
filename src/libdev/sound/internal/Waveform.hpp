/***********************************************************

  W A V E F O R M . H P P

***********************************************************/

/*

  Used as an interface to a waveform on disk.

  At the moment the only files it will work on are
  .wav files. From these it can get the data size, format
  and read the data off.

  As you can see from looking at the implementation
  windows multimedia I/O is used so any further
  supported formats would need to use another
  I/O scheme.

  isWaveFile is provided as static so you do not
  need to incur any initialisation overhead.

*/

#ifndef WAVEFORM_HPP
#define WAVEFORM_HPP

#include "base/base.hpp"
#include "system/PathName.hpp"
#include "sound/internal/WaveFormat.hpp"
#include "sound/WaveformId.hpp"

#include <audio/wave.h>
#include <al.h>

#include <memory>

////////////////////////////////////////////////////////////

class SndWaveform
{
public:
    SndWaveform(const SndWaveformId& id);
    ~SndWaveform();

    static bool isWaveFile(const char* path);

    WaveFormat* format();
    // returns size of PCM data in bytes
    uint dataSize();

    // reads all the wave data to the given buf and
    // returns the size of buf in bytes
    uint read(void* buf, uint from, uint len);

    // return the pathname of the SndWaveform
    const SysPathName& pathname() const;

    const SndWaveformId& id() const;

    SndWaveform& addReference();
    SndWaveform& removeReference();

    size_t ref() const;

    ALenum toALformat();

private:
    /** revoked operations **/
    SndWaveform();
    SndWaveform(const SndWaveform&);
    SndWaveform& operator=(const SndWaveform&);

    SndWaveformId id_;
    WaveInfo* waveInfo_;

    size_t ref_;
    uint dataSize_{}; // The cached size of the sound. 0 ==> not calculated yet
    std::unique_ptr<WaveFormat> pFormat_{};
};

////////////////////////////////////////////////////////////

#endif /* SndWaveform_HPP ************************************/
