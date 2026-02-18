#include "planeted/sdlapp.hpp"

#include "base/base.hpp"
#include "base/Diag.hpp"
#include "base/IProgressReporter.hpp"
#include "base/Error.hpp"
#include "gui/gui.hpp"
#include "sound/snd.hpp"
#include "sound/Mixer.hpp"
#include "sound/MixerParameters.hpp"
#include "device/ButtonEvent.hpp"
#include "mathex/Point2d.hpp"
#include "phys/phys.hpp"
#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include "device/Time.hpp"
#include "device/Mouse.hpp"
#include "render/Font.hpp"
#include "render/Painter.hpp"
#include "render/Display.hpp"
#include "render/Device.hpp"
#include "render/Surface.hpp"
#include "render/Capabilities.hpp"
#include "mathex/EulerAngles.hpp"
#include "world4d/Entity/Root.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Sound/SoundManager.hpp"
#include "world4d/Scene/Shadow.hpp"
#include "world4d/Scene/Camera.hpp"
#include "machlog/Actors/Actor.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Races.hpp"
#include "machlog/World/Scenario.hpp"
// #include "machlog/dbgstuff.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"
#include "machphys/Data/Data.hpp"
#include "machphys/Data/Preload.hpp"
#include "envirnmt/PlanetEnvironment.hpp"
#include "planeted/errorhnd.hpp"
#include "planeted/leaktrak.hpp"
#include "planeted/availods.hpp"
#include "planeted/planeted.hpp"
#include "planeted/cameras.hpp"
#include "sim/Manager.hpp"
#include "mathex/CoordSystem.hpp"
#include "system/Variable.hpp"
#include "system/WindowsAPI.hpp"

#include "machphys/machphys.hpp"
#include "machphys/Data/Levels.hpp"
#include "machphys/Persistence/Persistence.hpp"

class ProgressIndicator : public IProgressReporter
{
public:
    ProgressIndicator()
    {
        lowerLimit_ = 0.0;
        upperLimit_ = 1.0;
        lastDone_ = 0;
    }

    size_t report(size_t done, size_t maxDone) override
    {
        if (done == lastDone_)
            return 0;
        const double minx = 98;
        const double maxx = 538;
        const double miny = 362;
        const double maxy = 366;
        const double width = maxx - minx + 1;
        const double height = maxy - miny + 1;
        const double limitRange = upperLimit_ - lowerLimit_;
        const double percentComplete = (((double)done / (double)maxDone) * limitRange) + lowerLimit_;
        const double displayWidth = std::min((percentComplete * width) + 5, width);

        const double red = 255.0 / 255.0;
        const double green = 250.0 / 255.0;
        const double blue = 142.0 / 255.0;

        RenSurface frontBuf = RenDevice::current()->frontSurface();
        Ren::Painter frontPainter(frontBuf);
        frontPainter.filledRectangle(
            Ren::Rect(minx, miny, displayWidth, height),
            RenColour(red, green, blue));
        RenDevice::current()->flushCommandBuffer();
        RenDevice::current()->display()->flipBuffers();
        frontPainter.filledRectangle(
            Ren::Rect(minx, miny, displayWidth, height),
            RenColour(red, green, blue));
        lastDone_ = done;
        return (double)maxDone / 50.0;
    }

    void setLimits(double lower, double upper)
    {
        lowerLimit_ = lower;
        upperLimit_ = upper;
    }

private:
    double lowerLimit_;
    double upperLimit_;
    size_t lastDone_;
};

void debugTiming(const char* text, bool startTiming)
{
    static PhysAbsoluteTime startTime;
    static PhysAbsoluteTime endTime;
    if (startTiming)
    {
        // FAST_DEBUG( text << std::endl );
        startTime = Phys::time();
    }
    else
    {
        // FAST_DEBUG( text << std::endl );
        endTime = Phys::time();
        DEBUG_STREAM(DIAG_LIONEL, text << " , " << (endTime - startTime) << std::endl);
    }
}

SDLApp::SDLApp(int argc, char* argv[])
    : AfxSdlApp(argc, argv)
    , winWidth_(400)
    , winHeight_(300)
    , loadScenario_("NONE")
    , loadArtefact_("")
{
}

SDLApp::~SDLApp()
{
}

inline double degToRad(double degs)
{
    return degs / 180 * Mathex::PI;
}

static void usage()
{
    static string usageString;
    usageString = "Command line parameters are:\n";
    usageString += "\t<planet name>\n";
    usageString += "or\n";
    usageString += "\t<planet name> -f [<screen mode number>]\n";
    usageString += "or\n";
    usageString += "\t<planet name> -w <window width> <window height>\n";
    usageString += "\n";
    usageString += "Where:\n";
    usageString += "\t-w means run in window mode (the default)\n";
    usageString += "\t-f means run in full-screen mode\n";
    usageString += "\t<screen mode number> allows you to chose the screen resolution.\n";
    usageString += "\n";
    usageString += "Additional [optional] parameters are:\n";
    usageString += "\t-s < scenario file > ( .scn suffix is ADDED to filename)\n";
    usageString += "\t-a < artefact file > ( .arf suffix is ADDED to filename)";
    usageString += "\n";
    std::cout << usageString;
    std::cerr << usageString;
    SysWindowsAPI::messageBox(usageString.c_str(), "PlanetEditor v352.01");
}

static void newHandler()
{
    WHERE_STREAM("Aborting in new handler " << std::endl);
    // FAST_DEBUG("Aborting in new handler" << std::endl );
    ASSERT_FAIL("Aborting in new handler");
}

// Temporary timing stuff.
#ifndef PRODUCTION
#include "utility/DebugTimer.hpp"
#endif
// #include <env.h>

bool SDLApp::clientStartup()
{
    Config::initConfigManager();

    // Set coord system
    MexCoordSystem::instance().set(MexCoordSystem::LEFT_HANDED);
    ProgressIndicator progressIndicator;

    // The debug timer doesn't work if this environment var isn't set.
    // setenv("CHARYBDIS", "x", false);
    // UtlDebugTimer::startCalibration();
#ifndef PRODUCTION
    UtlDebugTimer::calibrate();
#endif

    HAL_STREAM("SDLApp::clientStartup\n");
    char* pRate = getenv("MACH_RATE");

    if (pRate)
    {
        MATHEX_SCALAR rate = atof(pRate);
        DevTime::instance().rate(rate);

        std::cout << "Rate is " << rate << std::endl;
    }

    char* pRunTime = getenv("MACH_RUN_TIME");

    if (pRunTime)
    {
        runTime_ = atof(pRunTime);

        std::cout << "Run time is " << runTime_ << std::endl;
    }

    //    set_new_handler( newHandler );

    // Set the diretory to look for all files
    SysPathName::rootEnvironmentVariable("MACH_ROOT");

    MachPhysData::instance();

    string planetName;
    bool windowMode = true;
    size_t displayMode = 0;
    std::optional<Ren::Size> modSize;

    switch (invokeArgs().size())
    {
        case 8: // <planet file> -w <width> <height> -s <scenario.scn> -a <artefact.arf>
                //<planet file> -f <width> <height> -s <scenario.scn> -a <artefact.arf>
            {
                loadArtefact_ = invokeArgs()[7].c_str();
            }
        // Deliberate Drop through!!!
        case 6: // <planet file> -w <width> <height> -s <scenario.scn>
                // <planet file> -f <width> <height> -s <scenario.scn>
            {
                loadScenario_ = invokeArgs()[5].c_str();
            }
        // Deliberate Drop through!!!
        case 4: // <planet file> -w <width> <height>
                // <planet file> -f <width> <height>
            {
                const string flag = invokeArgs()[1];
                windowMode = (flag == "-w");
                int modeW = atoi(invokeArgs()[2].c_str());
                int modeH = atoi(invokeArgs()[3].c_str());
                modSize = Ren::Size(modeW, modeH);
            }

        case 1:
            planetName = invokeArgs()[0];
            break;
        default:
            usage();
            return false;
    }

    ASSERT(planetName.length() > 0, logic_error());

    // This must be called before any render library object is created.  (It
    // ensures a correct destruction order for render library Singletons.)
    Ren::initialise();

    W4dRoot* root = pRoot_ = new W4dRoot(W4dRoot::W4dRootId());

    pDisplay_ = new RenDisplay(window());

    ErrorHandler::instance().pDisplay(pDisplay_);
    pDisplay_->buildDisplayModesList();

    const RenDisplay::Mode desktopMode = pDisplay_->getDesktopDisplayMode();

    if (windowMode)
    {
        Ren::Size windowSize = modSize.value_or(desktopMode.size() * 0.75f);
        const RenDisplay::Mode selectedMode = pDisplay_->getWindowedMode(windowSize.width, windowSize.height);
        pDisplay_->useMode(selectedMode);
    }
    else
    {
        // Initially, pick the lowest-res 16-bit mode.
        pDisplay_->useFullScreen();

        if (modSize->isNull())
            modSize = desktopMode.size();

        std::cout << "Trying to select display mode " << modSize->width << "x" << modSize->height << "x" << 16 << std::endl;
        const RenDisplay::Mode selectedMode = pDisplay_->findMode(modSize->width, modSize->height, 0);
        if (!selectedMode.isValid() || !pDisplay_->useMode(selectedMode))
        {
            std::cout << "Failed to select that mode -- the nearest alternative will be chosen." << std::endl;
            pDisplay_->useNearestMode(modSize->width * modSize->height, 16);
        }

        // If there are no 16-bit modes, then mode should default to the
        // first mode in the list, regardless of depth.
        std::cout << "Initially, setting mode: " << pDisplay_->currentMode() << std::endl;

        // Ask the mouse to give us coordinates scale to the current Direct3D
        // resolution (which doesn't necessarilly match the Windows resolution).
        const RenDisplay::Mode& mode = pDisplay_->currentMode();
        DevMouse::instance().scaleCoordinates(mode.width(), mode.height());
    }

    // Initialise

    // Ensure correct order of destruction of static singletons.
    // Destruction order is reverse of creation

    {
        const RenDisplay::Mode& mode = pDisplay_->currentMode();
        if (mode.width() > 1024 && mode.height() > 768)
        {
            Gui::setUiScaleFactor(2);
        }
        else
        {
            Gui::setUiScaleFactor(1);
        }
    }

    Ren::initFonts();
    RenSurface::setDefaultFontSize(10 * Gui::uiScaleFactor());

    std::unique_ptr<RenDevice> pDevice = std::make_unique<RenDevice>(pDisplay_);
    if (!pDevice->initialize())
        return false;

    manager_ = new W4dSceneManager(std::move(pDevice), root);
    manager_->pDevice()->debugTextCoords(4 * Gui::uiScaleFactor(), 4);

    W4dManager::instance().sceneManager(manager_);

    // Set the viewport boundary to the entire screen.
    RenDevice& device = *manager_->pDevice();
    const int w = device.windowWidth();
    const int h = device.windowHeight();
    device.setViewport(0, 0, w, h);

    // DevSound::instance();
    int nMaxSamples = 20;
    SndMixerParameters soundParams(
        //      window(), // Where this is the HWND of your application window
        Snd::Polyphony(5), // The Mixers polyphony
        SndMixerParameters::DIRECTSOUND, // You must be using DIRECTSOUND
        Snd::ELEVEN_THOUSAND_HZ, // The playback sample rate
        Snd::SIXTEEN_BIT, // The playback "bit-ness" ?!
        Snd::STEREO, // Speaker setup
        Snd::THREE_D, // THREE_D or TWO_D
        nMaxSamples // Number of samples that can exist at one time, an assertion will occur if this limit is
                    // overstepped.
    );

    SndMixer::initialise(soundParams);

    MachPhysPreload::registerSounds();

    W4dSoundManager::instance().readSoundDefinitionFile("sounds/snddef64.dat");

    SysPathName planetFileName;
    // Set up the texture search path.
    RenTexManager::PathNames searchList = RenTexManager::instance().searchList();

    // We set different search paths depending on the amount
    // of video memory availbable after the display mode has been set
    const bool canSupport4MegTexture = manager_->pDevice()->capabilities().supports4MBytesTextureSet();
    if (! canSupport4MegTexture)
    {
        searchList.push_back(SysPathName("models/texture2"));
        searchList.push_back(SysPathName("models/texture2/exp"));
        searchList.push_back(SysPathName("models/texture2/fire"));
        searchList.push_back(SysPathName("models/texture2/smoke"));
        planetFileName = planetPath(planetName, "texture2", &searchList);
    }
    else
    {
        searchList.push_back(SysPathName("models/texture4"));
        searchList.push_back(SysPathName("models/texture4/exp"));
        searchList.push_back(SysPathName("models/texture4/fire"));
        searchList.push_back(SysPathName("models/texture4/smoke"));
        planetFileName = planetPath(planetName, "texture4", &searchList);
    }

    RenTexManager::instance().searchList(searchList);
    // Used by PedTileEditor to cycle through the available *.lod file (tiles)
    SysPathName::Components lodPathComps;
    lodPathComps.push_back("models/planet");
    lodPathComps.push_back(planetName);
    lodPathComps.push_back("lod");
    SysPathName lodPath;
    lodPath.createFromComponents(lodPathComps);

    PedAvailableTileLods::instance().initialise(lodPath);

    // Display a loading screen.
    manager_->pDevice()->displayImage("gui/menu/wait.bmp");

    MachPhysPersistence::instance();

    // Construct the planet, surface and race
    pPlanet_ = &MachLogPlanet::instance();
    pPlanet_->surface(manager_, planetFileName, &progressIndicator);
    // Reuse loaded tiles lods
    pPlanet_->surface()->registerLoadedTiles();
    manager_->useLevelOfDetail(false);

    // If this environment variable is set, then don't load any Machines.
    // This provids a quick load when tuning sky colour, fog etc.
    if (!getenv("PLANET_ONLY"))
    {
        // size_t memBefore = DbgMemChkAllocationData().maxMemoryNewed();
        // memprobos << " Before redscar = " << memBefore << std::endl;
        MachLogRaces::instance();
    }
    HAL_STREAM("SDLApp::clientStartup readEnvironment\n");
    readEnvironment(planetName);

    manager_->hideStats();

    // Register the scene manager with the render libary manager
    HAL_STREAM("SDLApp::clientStartup sceneManager\n");

    W4dShadowProjected2d::shadowPlaneZ(0.025);

    SysPathName::Components cspPathComps;
    cspPathComps.push_back("models/planet");
    cspPathComps.push_back(planetName);
    cspPathComps.push_back(planetName + ".csp");
    SysPathName cspPath;
    cspPath.createFromComponents(cspPathComps);

    // Create csp file if it doesn't exist
    if (! cspPath.existsAsFile())
    {
        std::ofstream cspFile(cspPath.pathname().c_str());
    }
    MachCameras::instance().initialise(manager_, pRoot_);
    PedPlanetEditor::instance().initialise(manager_, pPlanet_);
    PedPlanetEditor::instance().readCspFile(cspPath);

    // Load artefact data ( if present )
    if (loadArtefact_ != "")
    {
        SysPathName::Components scnPathComps;
        scnPathComps.push_back("data");
        scnPathComps.push_back(loadArtefact_ + ".arf");
        SysPathName scnPath;
        scnPath.createFromComponents(scnPathComps);

        //  Abort if .arf doesn't exist
        if (! scnPath.existsAsFile())
        {
            string usageString = scnPath.c_str();
            usageString += "\t does not exist - terminating program.\n";
            std::cout << usageString;
            std::cerr << usageString;
            SysWindowsAPI::messageBox(usageString.c_str(), "PlanetEditor v0.01");
            exit(0);
        }
        else
        {
            PedPlanetEditor::instance().readArfFile(scnPath);
        }
    }

    // Load model data for constructions/machines
    if (!getenv("CB_NOPRELOAD"))
    {
        std::string modelsFile("pemodels.bin");
        if (!SysPathName(modelsFile).existsAsFile()) {
            modelsFile = "models.bin";
        }

        MachPhysPreload::persistentFileName(modelsFile);
        if (MachPhysPreload::persistentFileName().existsAsFile())
        {
            MachPhysPreload::persistentPreload(MachPhysPreload::ECHO_PROGRESS, &progressIndicator);
        }
        else
        {
            MachPhysPreload::swPreload(MachPhysPreload::ECHO_PROGRESS);
            MachPhysPreload::persistentSave(MachPhysPreload::ECHO_PROGRESS);
        }
    }

    if (loadScenario_ != "")
    {
        SysPathName::Components scnPathComps;
        scnPathComps.push_back("data");
        scnPathComps.push_back(loadScenario_ + ".scn");
        SysPathName scnPath;
        scnPath.createFromComponents(scnPathComps);

        // Create scn file if it doesn't exist
        if (! scnPath.existsAsFile())
        {
            std::ofstream scnFile(scnPath.pathname().c_str());
        }

        PedPlanetEditor::instance().readScnFile(scnPath);
    }

    initialised_ = true;
    finishTimer_.time(0);

    // UtlDebugTimer::finishCalibration();

    return true;
}

void SDLApp::clientShutdown()
{
    delete pEnvironment_;

    Ren::cleanUpFonts();

    DevMouse::instance().unhide();
    delete pRoot_;
    delete manager_;
    // Tell W4dManager about destruction of sceneManager
    W4dManager::instance().clearSceneManager();
    delete pDisplay_;

    Config::cleanUpConfigManager();
}

// Update the display.
void SDLApp::loopCycle()
{
    // Prevent processing before clientStartup() call
    if (! initialised_)
        return;

    SimManager::instance().cycle();

    processInput();

    // Update world4d
    W4dManager& w4dManager = W4dManager::instance();
    w4dManager.update();

    MachCameras::instance().updateCameras();

    // If time is suspended for profiling, make the general position ID
    // change, so that W4dEntity evaluates transforms is if time were advancing.
    static const bool timeSuspended = getenv("SUSPEND_TIME") != nullptr;
    if (timeSuspended)
        W4dManager::instance().generateGeneralPositionId();

    MexTransform3d xform = manager_->currentCamera()->globalTransform();

    PedPlanetEditor::instance().preRenderUpdate();

    if (manager_->pDevice()->startFrame())
    {
        // Render the scene
        debugTiming("SDLApp:: render", true);
        w4dManager.render();
        debugTiming("SDLApp:: render complete", false);

        manager_->pDevice()->endFrame();
    }

    MexPoint3d pos = xform.position();
}

void SDLApp::outputDebugInfo(const MexPoint2d& pos, const MexTransform3d& xform, MachActor* pActor)
{
    if (showPosition_)
    {
        MexEulerAngles angles;
        xform.rotation(&angles);
        int ez = (int)(angles.azimuth().asScalar() * 180.0 / Mathex::PI);
        int ey = (int)(angles.elevation().asScalar() * 180.0 / Mathex::PI);
        int ex = (int)(angles.roll().asScalar() * 180.0 / Mathex::PI);

        // Test the cursor to actor code
        if (pActor != nullptr)
            manager_->out() << "Cursor: " << pActor->objectType() << " "
                            << (void*)&((const MachActor*)pActor)->physObject() << std::endl;

        manager_->out() << "Eye at: " << pos << "(" << ez << "," << ey << "," << ex << ")\n";
    }

#ifndef NDEBUG
    if (showMemory_)
        manager_->out() << DbgMemChkAllocationData();
#endif

    if (aShowRace_[0])
    {
        MachLogRaces::instance().outputRace(manager_->out(), MachPhys::RED);
    }

    if (aShowRace_[1])
    {
        MachLogRaces::instance().outputRace(manager_->out(), MachPhys::BLUE);
    }

#ifndef NDEBUG
    // Update leak tracking
    LeakTracker::update(LeakTracker::INLOOP, LeakTracker::NONE);
#endif
}

// virtual
bool SDLApp::activate(/*WORD wordArg*/)
{
    if (!manager_)
        return false;

    if (manager_->pDevice())
        manager_->pDevice()->activate(/*wordArg*/);

    return true;
}

// virtual
void SDLApp::updateDisplay()
{
    if (initialised_ && manager_)
        manager_->updateDisplay();
}

// virtual
void SDLApp::getWindowSize(int&, int&, int& w, int& h)
{
    if (invokeArgs().size() > 1)
    {
        const string flag = invokeArgs()[1];
        if (flag == "-w" && invokeArgs().size() == 4)
        {
            winWidth_ = atoi(invokeArgs()[2].c_str());
            winHeight_ = atoi(invokeArgs()[3].c_str());
        }
    }

    w = winWidth_;
    h = winHeight_;
}

SysPathName
SDLApp::planetPath(const string& planetName, const string& texDirectory, RenTexManager::PathNames* pSearchList)
{
    string texturePath = "models/planet/";
    texturePath += planetName;
    texturePath += "/";
    texturePath += texDirectory;

    string psfPath = "models/planet/";
    psfPath += planetName;
    psfPath += "/";
    psfPath += planetName;
    psfPath += ".psf";

    pSearchList->push_back(SysPathName(texturePath));

    return SysPathName(psfPath);
}

void SDLApp::readEnvironment(const string& planetLeafName)
{
    PRE(manager_);

    string planetPath = "models/planet/";
    planetPath += planetLeafName;
    planetPath += "/";
    planetPath += planetLeafName;
    planetPath += ".env";
    SysPathName withExt = planetPath;
    ASSERT_INFO(withExt);
    ASSERT(withExt.existsAsFile(), "Environment file doesn't exist.");
    ALWAYS_ASSERT(withExt.existsAsFile(), ("Environment file doesn't exist." + withExt.filename()).c_str());

    // Create an environment for the sky etc
    // pEnvironment_ = new EnvPlanetEnvironment(withExt, manager_);
    // manager_->environment(pEnvironment_);
}

// virtual
void SDLApp::checkForQuit(const DevButtonEvent& devButtonEvent)
{
    if (devButtonEvent.scanCode() == Device::KeyCode::KEY_Q
        || (devButtonEvent.scanCode() == Device::KeyCode::ESCAPE && devButtonEvent.wasShiftPressed()))
    {
        finish();
        PhysConfigSpace2d* pConfigSpace = &pPlanet_->configSpace();

        std::ofstream os("pedconfig.log");
        os << *pConfigSpace;
        pConfigSpace->traceObstacles(os);
    }
}

/* End SDLApp.CPP ***************************************************/
