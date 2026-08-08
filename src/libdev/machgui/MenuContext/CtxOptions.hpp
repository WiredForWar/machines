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

    // What the display drop downs are showing.
    Ren::WindowMode selectedWindowMode() const;
    Ren::Size selectedResolution() const;
    int selectedRefreshRate() const;

    void setDisplaySettings(Ren::WindowMode windowMode, Ren::Size resolution, int refreshRate);

    // Bring the resolution and refresh rate drop downs into line with the window
    // mode, and each other. Only a mode we set ourselves leaves either of them ours
    // to choose.
    void updateDisplayControls();
    void updateResolutions();
    void updateRefreshRates();

    int indexOfResolution(Ren::Size resolution) const;

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
    MachGuiDropDownListBoxCreator* pWindowMode_{};
    MachGuiDropDownListBoxCreator* pScreenSize_{};
    MachGuiDropDownListBoxCreator* pRefreshRate_{};

    // What the resolution and refresh rate drop downs offer, in the order they
    // offer them.
    ctl_vector<Ren::Size> resolutions_;
    ctl_vector<int> refreshRates_;

    // Retained so that the display settings can be put back when the player cancels.
    Ren::WindowMode windowMode_{};
    Ren::Size resolution_;
    int refreshRate_{};

    // Set while the display drop downs are being brought into line, so that the
    // changes made to one do not send the others round again.
    bool updatingDisplayControls_ = false;

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
