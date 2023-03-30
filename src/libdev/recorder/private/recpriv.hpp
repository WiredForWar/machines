/*
 * R E C P R I V . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    RecRecorderPrivate

    This is the interface layer for the record / playback
    functions. This layer will change if the items that need
    to be recorded change.

    Libraries that need to do the recording process will require
    access to this header.
*/

#ifndef _RECORDER_RECPRIV_HPP
#define _RECORDER_RECPRIV_HPP

#include "base/base.hpp"
#include "device/butevent.hpp"
#include "device/keyboard.hpp"
#include "system/registry.hpp"

#include "sound/snd.hpp"

#include "utility/DependencyProvider.hpp"

#include <string>
#include <utility>

class MexPoint3d;
using DevMousePosition = std::pair<int32, int32>;

class RecRecorderPrivate
// Canonical form revoked
{
public:
    //  Singleton class
    static RecRecorderPrivate& instance();
    ~RecRecorderPrivate();

    //  Recording functions -------------------------------------

    void recordTime(double time);
    void recordMessageTime(double time);

    void recordMousePosition(const DevMousePosition&);
    void recordLeftButton(bool);
    void recordRightButton(bool);

    void recordKey(bool);
    void recordKeyCode(bool);
    void recordAnyKey(bool);
    void recordShiftPressed(bool);
    void recordCtrlPressed(bool);
    void recordAltPressed(bool);
    void recordDeltaKeyCode(const DevKeyboard::KeyState&);

    void recordButtonEvent(const DevButtonEvent&);
    void recordEventQueueLength(size_t);

    void recordRandomSeed(uint32_t);

    void recordSoundIsActive(bool);
    void recordSoundIsAudible(bool);
    void recordSoundIs3dSound(bool);
    void recordSoundSamplePosition(const MexPoint3d&);
    void recordSoundSampleVolume(Snd::Volume);
    void recordSoundNoOfFreeLogicalChannels(uint32_t);
    void recordSoundNoOfUsedLogicalChannels(uint32_t);

    void recordAniSmackerFinished(bool);

    void recordRegistryReturnValue(SysRegistry::ReturnValue);
    void recordRegistryKey(SysRegistry::KeyHandle);
    void recordRegistryBuffer(const void* pBuffer, int bufferSize);
    void recordRegistryStringValue(const std::string&);
    void recordRegistryIntegerValue(int);

    void recordNetworkInt(int);
    void recordNetworkUint(uint);
    void recordNetworkData(const uint8* buffer, size_t bufferSize);
    void recordNetworkBool(bool);
    void recordNetworkString(const std::string&);
    void recordNetworkDouble(double);

    //  Playback functions --------------------------------------
    double playbackTime() const;
    double playbackMessageTime() const;

    DevMousePosition playbackMousePosition() const;
    bool playbackLeftButton() const;
    bool playbackRightButton() const;

    bool playbackKey() const;
    bool playbackKeyCode() const;
    bool playbackAnyKey() const;
    bool playbackShiftPressed() const;
    bool playbackCtrlPressed() const;
    bool playbackAltPressed() const;
    DevKeyboard::KeyState playbackDeltaKeyCode() const;

    DevButtonEvent playbackButtonEvent() const;
    size_t playbackEventQueueLength() const;

    uint32_t playbackRandomSeed() const;

    bool playbackSoundIsActive() const;
    bool playbackSoundIsAudible() const;
    bool playbackSoundIs3dSound() const;
    MexPoint3d playbackSoundSamplePosition() const;
    Snd::Volume playbackSoundSampleVolume() const;
    uint32_t playbackSoundNoOfFreeLogicalChannels() const;
    uint32_t playbackSoundNoOfUsedLogicalChannels() const;

    bool playbackAniSmackerFinished() const;

    SysRegistry::ReturnValue playbackRegistryReturnValue() const;
    SysRegistry::KeyHandle playbackRegistryKey() const;
    int playbackRegistryBuffer(void* pBuffer) const;
    std::string playbackRegistryStringValue() const;
    int playbackRegistryIntegerValue() const;

    int playbackNetworkInt() const;
    uint playbackNetworkUint() const;
    void playbackNetworkData(uint8* buffer, size_t bufferSize) const;
    bool playbackNetworkBool() const;
    std::string playbackNetworkString() const;
    double playbackNetworkDouble() const;

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const RecRecorderPrivate& t);

    RecRecorderPrivate(const RecRecorderPrivate&);
    RecRecorderPrivate& operator=(const RecRecorderPrivate&);

    RecRecorderPrivate();
};

/* *******************************************************
 * SINGLETON DEPENDENCY PROVIDER
 */
template <> inline RecRecorderPrivate& DependencyProvider<RecRecorderPrivate>::getProvided()
{
    return RecRecorderPrivate::instance();
}

#ifdef _INLINE
#include "recorder/private/recpriv.itf"
#endif

#ifdef _INSTANTIATE_TEMPLATE_FUNCTIONS
#include "recorder/private/recpriv.ctf"
#endif

#endif

/* End RECPRIV.HPP **************************************************/
