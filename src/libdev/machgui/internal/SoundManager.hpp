/*
 * M G S N D M A N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiSoundManager

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_MGSNDMAN_HPP
#define _MACHGUI_MGSNDMAN_HPP

#include "base/base.hpp"
#include "ctl/List.hpp"
#include "sound/SampleHandle.hpp"

class SysPathName;

class MachGuiSoundManager
// Canonical form revoked
{
public:
    // The maximum number of sounds that can be playing
    // at a single time
    static constexpr int MAX_SOUNDS = 3;

    //  Singleton class
    static MachGuiSoundManager& instance();
    ~MachGuiSoundManager();

    // Play the requested sound
    // Return value as to whether the sound
    // will be played or not
    bool playSound(const SysPathName& wavFilePath);
    // PRE(wavFilePath.existsAsFile());

    // Update method must be called fairly regularly
    // to ensure sound resources are updated
    void update();

    // Reset the sound manager, stop and clear any playing
    // samples.
    void clearAll();

    // Just after loading a game it sometimes takes time for the gui to "settle down". i.e.
    // buttons are being set to default positions etc. You don't want sounds triggering off
    // when this is happening.
    void delaySounds();

    void CLASS_INVARIANT;

private:
    using SampleHandleList = ctl_list<SndSampleHandle>;

    SampleHandleList currentActiveHandles_;
    int delaySoundsFrameCount_ = 2;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiSoundManager& t);

    MachGuiSoundManager(const MachGuiSoundManager&) = delete;
    MachGuiSoundManager& operator=(const MachGuiSoundManager&) = delete;

    MachGuiSoundManager();
};

#endif

/* End MGSNDMAN.HPP *************************************************/
