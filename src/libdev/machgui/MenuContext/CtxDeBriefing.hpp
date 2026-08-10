/*
 * C T X D E B R F . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxDeBriefing

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXDEBRF_HPP
#define _MACHGUI_CTXDEBRF_HPP

#include "machgui/GameMenuContext.hpp"

#include "sound/Mixer.hpp"

class GuiImage;

class MachGuiCtxDeBriefing : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxDeBriefing(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxDeBriefing() override;

    void CLASS_INVARIANT;

    void update() override;

    bool okayToSwitchContext() override;

    void buttonEvent(MachGui::ButtonEvent) override;

protected:
    void playDeBriefingVoicemail();
    void stopPlayingDeBriefingVoicemail();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxDeBriefing& t);

    MachGuiCtxDeBriefing(const MachGuiCtxDeBriefing&) = delete;
    MachGuiCtxDeBriefing& operator=(const MachGuiCtxDeBriefing&) = delete;

    void displayDeBriefImage();

    // Data members...
    GuiImage* pDebriefImage_;
    SndSampleHandle debriefVoicemail_;
    // Has this mail been played yet
    bool playedMail_;
};

#endif

/* End CTXDEBRF.HPP *************************************************/
