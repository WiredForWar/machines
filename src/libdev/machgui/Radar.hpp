/*
 * R A D A R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiRadar

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_RADAR_HPP
#define _MACHGUI_RADAR_HPP

#include "base/base.hpp"
#include "gui/Displayable.hpp"
#include "gui/gui.hpp"

class MachActor;
class MachLog1stPersonHandler;

class MachGuiRadar : public GuiDisplayable
// Canonical form revoked
{
public:
    MachGuiRadar(GuiDisplayable* pParent, const Gui::Coord& relPos);
    ~MachGuiRadar() override;

    void CLASS_INVARIANT;

    void actor(MachActor*);
    void resetActor();

    void logHandler(MachLog1stPersonHandler*);
    void resetLogHandler();

    void initialise();

    void loadBitmaps();

    void unloadBitmaps();

protected:
    void doDisplay() override;

    void displayHealthArmour();
    void displayRadarBlips();
    void displayMotionDirection();
    void displayAnimatedRadarFrame();

    static GuiBitmap* machineImage();
    static GuiBitmap* constructionImage();
    static GuiBitmap* podImage();
    static GuiBitmap* missileEmplacementImage();
    static GuiBitmap& debrisImage();
    static GuiBitmap& artefactImage();
    static GuiBitmap& oreImage();
    static GuiBitmap* arrowImage();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiRadar& t);

    MachGuiRadar(const MachGuiRadar&);
    MachGuiRadar& operator=(const MachGuiRadar&);

    // Data members...
    static constexpr int RADAR_ANIMATION_FRAMES = 10;

    MachActor* pActor_{};
    GuiBitmap healthBmp_[3];
    GuiBitmap armourBmp_[3];
    GuiBitmap radarBackdropBmp_;
    GuiBitmap radarDomeBmp_;
    MachLog1stPersonHandler* pLogHandler_{};
    bool justEnteredFirstPerson_{};
    GuiBitmap radarStartupFrames_[RADAR_ANIMATION_FRAMES];
    GuiBitmap machineIcon_;
    bool hpAboveCritical_{};
    double animationEndTime_{};
    int frameNumber_{};
};

#endif

/* End RADAR.HPP ****************************************************/
