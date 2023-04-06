/*
 * T I M E . H P P
 * (c) Charybdis Limited, 1995. All Rights Reserved
 */
// Include this file to access the time under any supported
// operating system.  The actual choice of time implementation
// must be compiled into the client code, all implementations
// should be source code compatible.  You must of course link
// with the corresponding library.

#ifndef DEVICE_TIME_HPP_INCLUDED
#define DEVICE_TIME_HPP_INCLUDED

#include "base/base.hpp"
#include "utility/DependencyProvider.hpp"

// Provide access to time, measured in seconds.  Also supported are
// the ability to pause & restart time and to slow down or speed up
// time by a multiplier.
class DevTime
{
public:
    static DevTime& instance();

    double time() const;
    double resolution() const;

    // Multiply time by a scalar, so we can make it proceed faster
    // or slower than real-time as maintained by the OS.
    void rate(double rate);
    double rate() const;

    // Start and stop time.
    void pause();
    void resume();

    ~DevTime();

private:
    //  Singleton class
    DevTime();

    //  The "NoRecord" function is only suppied to allow the sound library
    //  to work without making any non-repeatable recorded calls to the
    //  timers.
    friend class DevTimer;
    double timeNoRecord() const;

    double rate_;
    bool paused_;
    double offset_;
    double pausedValue_;

// This include defines a private, nested class called OSTime.
#if defined _SDLAPP
#include "device/private/sdltime.hpp"
#else
#error The platform specific app type must be defined.
#endif

    // This member provides the wall-clock time maintained by the OS.
    OSTime OSTime_;

    // Operations deliberately revoked
    DevTime(const DevTime&);
    const DevTime& operator=(const DevTime&);
    bool operator==(const DevTime&) const;
};

/* *******************************************************
 * SINGLETON DEPENDENCY PROVIDER
 */
template <> inline DevTime& DependencyProvider<DevTime>::getProvided()
{
    return DevTime::instance();
}

#endif
/* End TIME.HPP ****************************************************/
