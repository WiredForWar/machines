/*
 * C T X O P T N S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxoptions

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXOPTNS_HPP
#define _MACHGUI_CTXOPTNS_HPP

#include "machgui/GameMenuContext.hpp"

#include "ctl/PtrVector.hpp"
#include "ctl/CountedPtr.hpp"
#include "ctl/Vector.hpp"
#include "machgui/StartupScreens.hpp"
#include "render/Display.hpp"

class MachGuiSlideBar;
class MachGuiDropDownListBoxCreator;
class MachGuiCheckBox;

namespace MachGui
{

enum class VSyncMode;

} // namespace MachGui

class MachGuiCtxOptions : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxOptions(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxOptions() override;

    void CLASS_INVARIANT;

    void update() override;
    bool okayToSwitchContext() override;
    void buttonEvent(MachGui::ButtonEvent) override;

    void exitFromOptions();

    static void load3dSoundFiles(bool enabled);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxOptions& t);

    void writeToConfig();
    void readFromConfig();

    // The mode the screen size drop down is showing, or nullptr when it is showing
    // none.
    const RenDisplay::Mode* selectedScreenMode() const;

    MachGuiCtxOptions(const MachGuiCtxOptions&);
    MachGuiCtxOptions& operator=(const MachGuiCtxOptions&);

    using BooleanOptimisations = ctl_pvector<MachGuiCheckBox>;
    using ChoicesOptimisations = ctl_pvector<MachGuiDropDownListBoxCreator>;

    // Data members...
    MachGuiStartupScreens::Context exitContext_;

    MachGuiSlideBar* pMusicVolume_{};
    MachGuiSlideBar* pSoundVolume_{};
    MachGuiCheckBox* pSound3d_{};
    MachGuiCheckBox* pTransitions_{};
    MachGuiCheckBox* pCursorType_{};
    MachGuiCheckBox* pReverseKeys_{};
    MachGuiCheckBox* pReverseMouse_{};
    MachGuiCheckBox* pGrabMouse_{};
    MachGuiCheckBox* pWasdControls_{};
    MachGuiDropDownListBoxCreator* pScreenSize_{};

    // The modes the screen size drop down offers, in the order it offers them.
    ctl_vector<const RenDisplay::Mode*> screenModes_;

    BooleanOptimisations booleanOptimisations_;
    ChoicesOptimisations choicesOptimisations_;
    float musicVolume_, soundVolume_;
    bool use2DMarker_{};
    bool grabCursor_{};
    int32_t zenithCameraAcceleration_{};
    int32_t groundCameraAcceleration_{};
    MachGui::VSyncMode vsyncMode_{};
    MachGuiDropDownListBoxCreator* vSyncModeDropDown_{};
    MachGuiDropDownListBoxCreator* pScaleFactorSelector_{};
    bool exitFromOptions_ = false;
    bool cursorType2d_;
    MachGuiSlideBar* pCameraAccelerationSlider_{};
};

#endif

/* End CTXOPTNS.HPP *************************************************/
