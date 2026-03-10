/***********************************************************

  W A V E F O R M . C P P

***********************************************************/

#include "sound/internal/Waveform.hpp"
#include "sound/internal/WaveFormat.hpp"
#include "system/PathName.hpp"
#include "system/VFS.hpp"

////////////////////////////////////////////////////////////

/** STATIC **/
bool SndWaveform::isWaveFile(const char* path)
{
    bool isGood = false;
    if (!path)
        return isGood;

    SysPathName temp(path);

    isGood = temp.existsAsFile();

    if (isGood)
    {
        WaveInfo* wave;
        wave = WaveOpenFileForReading(path);
        if (!wave)
        {
            isGood = false;
        }
        else
        {
            isGood = !WaveSeekFile(0, wave);
            WaveCloseFile(wave);
        }
    }
    return isGood;
}

////////////////////////////////////////////////////////////

SndWaveform::SndWaveform(const SndWaveformId& id)
    : id_(id)
    , ref_(1)
{
    PRE(SndWaveform::isWaveFile(id_.pathname().c_str()));

    const std::string fileName = System::findFile(id.pathname().pathname());
    waveInfo_ = WaveOpenFileForReading(fileName.c_str());
    POST(waveInfo_);
}

SndWaveform::~SndWaveform()
{
    WaveCloseFile(waveInfo_);
}

////////////////////////////////////////////////////////////

WaveFormat* SndWaveform::format()
{
    if (!pFormat_)
    {
        pFormat_ = std::make_unique<WaveFormat>(
            (Channels)waveInfo_->channels,
            (SampleRateHz)waveInfo_->sampleRate,
            (BitsPerSample)waveInfo_->bitsPerSample);
    }

    return pFormat_.get();
}

uint SndWaveform::dataSize()
{
    // Ensure the size is cached
    if (dataSize_ == 0)
    {
        dataSize_ = waveInfo_->dataSize;
    }

    return dataSize_;
}

ALenum SndWaveform::toALformat()
{
    bool stereo = (waveInfo_->channels > 1);

    switch (waveInfo_->bitsPerSample)
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

////////////////////////////////////////////////////////////

// this reads directly to the passed
// buf pointer. from specifies an offset into the
// data and len specifies the size of the buf array.
// the function returns the number of bytes read
uint SndWaveform::read(void* buf, uint from, uint len)
{
    /*  MMCKINFO dataChunk, chunk;
    uint nBytesRead =0;

    mmioSeek( hmmio_, 0, SEEK_SET );

    // initialise chunks
    chunk.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );
    dataChunk.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );

    mmioDescend( hmmio_, &chunk, NULL, MMIO_FINDRIFF );

    mmioDescend( hmmio_, &dataChunk, &chunk, MMIO_FINDCHUNK );

    mmioSeek( hmmio_, from, SEEK_CUR );

    nBytesRead = mmioRead( hmmio_, (HPSTR)buf, (LONG)len );

    // go to begining of file
    mmioSeek( hmmio_, 0, SEEK_SET );*/

    uint nBytesRead = 0;
    WaveSeekFile(from, waveInfo_);

    nBytesRead = WaveReadFile((char*)buf, len, waveInfo_);
    WaveSeekFile(from, waveInfo_);

    return nBytesRead;
}

////////////////////////////////////////////////////////////

const SysPathName& SndWaveform::pathname() const
{
    return id_.pathname();
}

////////////////////////////////////////////////////////////

SndWaveform& SndWaveform::addReference()
{
    ++ref_;
    return *this;
}

////////////////////////////////////////////////////////////

SndWaveform& SndWaveform::removeReference()
{
    --ref_;
    return *this;
}

size_t SndWaveform::ref() const
{
    return ref_;
}

/* SndWaveformINFO.CPP ***************************************/
