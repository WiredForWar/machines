#ifndef _SDLAPP_HPP
#define _SDLAPP_HPP

#include "system/pathname.hpp"
#include "device/timer.hpp"
#include "device/butevent.hpp"
#include "render/texmgr.hpp"
#include "afx/AfxSdlApp.hpp"

// class DevButtonEvent;
class RenDisplay;
class W4dEntity;
class W4dSceneManager;
class W4dRoot;
class MachLogPlanet;
class MachActor;
class MachCameras;
class GuiRoot;
class MexTransform3d;
class MexPoint2d;
class MexPoint3d;
class EnvPlanetEnvironment;

// This class represents an application based on Direct3D.  When we know
// what's common to all D3D apps, this probably ought to become an abstract
// base class.  It's based on the application framework, so there's no main,
// just this class.
class SDLApp : public AfxSdlApp
{
public:
    SDLApp(int argc, char* argv[]);
    ~SDLApp() override;

private:
    friend class AfxSingletonApp;

    // Provide virtual fns required by base classes.
    void clientShutdown() override;
    void loopCycle() override;
    void updateDisplay() override;
    //  virtual bool activate(WORD wParam);
    virtual bool activate(/*WORD wParam*/);
    bool clientStartup() override;

    void processInput();
    void checkForCameraChange(const DevButtonEvent&);
    void checkForQuit(const DevButtonEvent&);

    void moveObject(W4dEntity* pObject);

    SysPathName planetPath(const string& planetName, const string& texName, RenTexManager::PathNames* pSearchList);
    void readEnvironment(const string& planetName);

    // Allow a derived class to override this and provide a window size.
    // This shouldn't be necessary when DevDisplay works with Win95 & D3D.
    virtual void getWindowSize(int& ox, int& oy, int& w, int& h);

    void outputDebugInfo(const MexPoint2d& pos, const MexTransform3d& xform, MachActor* pActor);

    int winWidth_, winHeight_;
    double runTime_;
    MachLogPlanet* pPlanet_;
    RenDisplay* pDisplay_;
    W4dRoot* pRoot_;
    W4dSceneManager* manager_;
    bool showPosition_, showCurrentMachine_, showMemory_;
    bool aShowRace_[4];
    bool initialised_;
    DevTimer keyTimer_;
    DevTimer finishTimer_;
    string loadScenario_;
    string loadArtefact_;
    EnvPlanetEnvironment* pEnvironment_;
};

#endif
