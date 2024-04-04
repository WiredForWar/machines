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

#include "ctl/pvector.hpp"
#include "ctl/countptr.hpp"
#include "render/driv.hpp"
#include "machgui/startup.hpp"

#define GAMMA_REG_MULTIPLIER 0x19999999
#define GAMMA_LOWER_LIMIT 0.1
#define GAMMA_UPPER_LIMIT 5.0

class RenDriverSelector;
class MachGuiSlideBar;
class MachGuiDropDownListBoxCreator;
class MachGuiDDrawDropDownListBoxCreator;
class MachGuiCheckBox;
class RenDisplay;

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
    void showDirect3DDrivers();

    MachGuiCtxOptions(const MachGuiCtxOptions&);
    MachGuiCtxOptions& operator=(const MachGuiCtxOptions&);

    using BooleanOptimisations = ctl_pvector<MachGuiCheckBox>;
    using ChoicesOptimisations = ctl_pvector<MachGuiDropDownListBoxCreator>;

    friend class MachGuiDDrawDropDownCallback;

    // Data members...
    MachGuiStartupScreens::Context exitContext_;

    MachGuiSlideBar* pMusicVolume_ = nullptr;
    MachGuiSlideBar* pSoundVolume_ = nullptr;
    MachGuiCheckBox* pSound3d_ = nullptr;
    MachGuiCheckBox* pTransitions_ = nullptr;
    MachGuiCheckBox* pScreenResolutionLock_ = nullptr;
    MachGuiCheckBox* pCursorType_ = nullptr;
    MachGuiCheckBox* pReverseKeys_ = nullptr;
    MachGuiCheckBox* pReverseMouse_ = nullptr;
    MachGuiCheckBox* pGrabMouse_{};
    MachGuiDropDownListBoxCreator* pScreenSize_ = nullptr;
    BooleanOptimisations booleanOptimisations_;
    ChoicesOptimisations choicesOptimisations_;
    float musicVolume_, soundVolume_, gammaCorrection_;
    RenDriverSelector* pDriverSelector_;
    MachGuiDDrawDropDownListBoxCreator* pDirectDrawDrivers_ = nullptr;
    MachGuiDropDownListBoxCreator* pDirect3DDrivers_ = nullptr;
    MachGuiDropDownListBoxCreator* pScaleFactorSelector_ = nullptr;
    bool exitFromOptions_ = false;
    RenDriverPtr initialDDrawDriver_;
    bool cursorType2d_;
    MachGuiSlideBar* pGammaCorrection_ = nullptr;
};

#endif

/* End CTXOPTNS.HPP *************************************************/
