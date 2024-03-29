/*
 * W 9 5 T I M E . C P P
 * (c) Charybdis Limited, 1995. All Rights Reserved
 */

#include "device/time.hpp"
#include "recorder/recorder.hpp"
#include "recorder/private/recpriv.hpp"
#include "device/internal/sdltimei.hpp"
#include <SDL.h>

// TODO TIMER
double DevTime::OSTime::resolution() const
{
    return DevSdlTimeInternal::instance().resolution();
}

double DevTime::OSTime::time() const
{
    double result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackTime();
    else
    {
        result = timeNoRecord();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordTime(result);
    }

    return result;
}

double DevTime::OSTime::timeNoRecord() const
{
    double result;
    // result = SDL_GetTicks() / 1000.0;
    result = resolution() * SDL_GetTicks();

    return result;
}
