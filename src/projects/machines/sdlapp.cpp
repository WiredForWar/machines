// #include <env>

#include "machines/sdlapp.hpp"

#include "base/IProgressReporter.hpp"
#include "base/diag.hpp"
#include "system/pathname.hpp"
#include "system/winapi.hpp"
#include "system/registry.hpp"
#include "mathex/point2d.hpp"
#include "mathex/eulerang.hpp"
#include "mathex/coordsys.hpp"
#include "device/time.hpp"
#include "device/mouse.hpp"
#include "device/cd.hpp"
#include "network/netnet.hpp"
#include "network/node.hpp"
#include "profiler/profiler.hpp"
#include "phys/phys.hpp"
#include "render/TextOptions.hpp"
#include "render/display.hpp"
#include "render/device.hpp"
#include "render/capable.hpp"
#include "render/texmgr.hpp"
#include "render/texture.hpp"
#include "render/texset.hpp"
#include "world4d/root.hpp"
#include "world4d/scenemgr.hpp"
#include "world4d/manager.hpp"
#include "world4d/shadow.hpp"
#include "machphys/mphydata.hpp"
#include "machphys/persist.hpp"
#include "machphys/preload.hpp"
#include "machlog/races.hpp"
#include "machlog/actor.hpp"
#include "machlog/network.hpp"
#include "machines/errorhnd.hpp"
#include "machines/leaktrak.hpp"
#include "render/colour.hpp"
#include "system/metafile.hpp"
#include "machgui/ctxoptns.hpp"
#include "machgui/gui.hpp"

#include "SDL_version.h"

#include "spdlog/spdlog.h"
#include <cstdio>

void debugTiming(const char* text, bool startTiming)
{
    static PhysAbsoluteTime startTime;
    static PhysAbsoluteTime endTime;
    if (startTiming)
    {
        startTime = Phys::time();
    }
    else
    {
        endTime = Phys::time();
        DEBUG_STREAM(DIAG_LIONEL, text << " , " << (endTime - startTime) << std::endl);
    }
}

SDLApp::SDLApp(int argc, char* argv[])
    : AfxSdlApp(argc, argv)
    , winWidth_(400)
    , winHeight_(300)
    , pDisplay_(nullptr)
    , pRoot_(nullptr)
    , pKeyboardFocus_(nullptr)
    , manager_(nullptr)
    , showMemory_(false)
    , initialised_(false)
    , pTextureSet_(nullptr)
    , savedSampleVolume_(0)
    , savedCDVolume_(0)
{
    for (int i = 0; i < 4; ++i)
        aShowRace_[i] = false;
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
    static string usage = "";
    usage += "Command line parameters are:\n";
    usage += "\t-?\tDisplay this help.\n";
    usage += "or\n";
    usage += "\t-f <xRes> <yRes>";
    usage += "or\n";
    usage += "\t-w <window width> <window height>\n";
    usage += "Where:\n";
    usage += "\t-w means run in window mode\n";
    usage += "\t-f means run in full-screen mode (default)\n";
    usage += "Leaving the command line blank will ask machines to default the resolution.";
    SysWindowsAPI::messageBox(usage.c_str(), "Machines");
}

static void newHandler()
{
    WHERE_STREAM("Aborting in new handler " << std::endl);
    ASSERT_FAIL("Aborting in new handler");
}

class ProgressIndicator : public IProgressReporter
{
public:
    ProgressIndicator(int xOffset, int yOffset)
        : upperLimit_(1.0)
        , xOffset_(xOffset)
        , yOffset_(yOffset)
    {
    }

    size_t report(size_t done, size_t maxDone) override
    {
        if (done == lastDone_)
            return 0;
        const double minx = 98 + xOffset_;
        const double maxx = 538 + xOffset_;
        const double miny = 362 + yOffset_;
        const double maxy = 366 + yOffset_;
        const double width = maxx - minx + 1;
        const double height = maxy - miny + 1;
        const double limitRange = upperLimit_ - lowerLimit_;
        const double percentComplete = (((double)done / (double)maxDone) * limitRange) + lowerLimit_;
        const double displayWidth = std::min((percentComplete * width) + 5, width);

        const double red = 255.0 / 255.0;
        const double green = 250.0 / 255.0;
        const double blue = 142.0 / 255.0;

        RenDevice::current()->frontSurface().filledRectangle(
            Ren::Rect(minx, miny, displayWidth, height),
            RenColour(red, green, blue));
        RenDevice::current()->display()->flipBuffers();
        // For double buffering do it twice to prevent bar from blinking
        RenDevice::current()->frontSurface().filledRectangle(
            Ren::Rect(minx, miny, displayWidth, height),
            RenColour(red, green, blue));
        // If the game session has come out prematurely then the network connection may have been reset.
        if (MachLogNetwork::instance().isNetworkGame())
        {
            MachLogNetwork::instance().update();
            SysWindowsAPI::sleep(0);
            SysWindowsAPI::peekMessage();
            // network game _may_ have come out on update above.
            /*              if( MachLogNetwork::instance().isNetworkGame() and
            MachLogNetwork::instance().node().lastPingAllTime() > 1 )
            {
                MachLogNetwork::instance().node().pingAll();
            }*/
        }
        lastDone_ = done;
        return (double)maxDone / 50.0;
    }

    void setLimits(double lower, double upper)
    {
        lowerLimit_ = lower;
        upperLimit_ = upper;
    }

private:
    double lowerLimit_ = 0;
    double upperLimit_ = 0;
    size_t lastDone_;
    int xOffset_ = 0;
    int yOffset_ = 0;
};

void SDLApp::initMusic()
{
    if (DevCD::instance().isAudioCDPresent())
    {
        // This is not a particularly nice place to do this initialisation
        // but if we want the music to start playing at the right volume
        // this is where it's going to happen
        SysRegistry::KeyHandle handle;
        if (SysRegistry::instance().onlyOpenKey("Options\\CD", &handle) == SysRegistry::SUCCESS)
        {
            size_t theVol = SysRegistry::instance().queryIntegerValue("Options\\CD", "Volume");
            DevCD::instance().volume(theVol);
        }
        // Play loading music. It's in a different position depending upon which CD is in.
        if (MachGui::machinesCDIsAvailable(1))
        {
            DevCD::instance().play(2); // Data 0, Menu Music 1, Loading Music 2
        }
        else if (MachGui::machinesCDIsAvailable(2))
        {
            DevCD::instance().play(4); // Data 0, Menu Music 1, Victory Music 2, Defeat Music 3, Loading Music 4
        }
    }
}

bool SDLApp::clientStartup()
{
    double time = DevTime::instance().time();
    double loadPos = 0;

    // Hide the cursor
    DevMouse::instance().hide();

// must be called before the display is created
// and the CD volume registry is used
#ifndef DEMO
    SysRegistry::instance().currentStubKey("SOFTWARE\\Acclaim Entertainment\\Machines");
#else
    SysRegistry::instance().currentStubKey("SOFTWARE\\Acclaim Entertainment\\Machines Demo");
#endif

#ifdef DEMO
    SysMetaFile::encryptionType(SysMetaFile::ENCRYPTION_2);
#else
    SysMetaFile::encryptionType(SysMetaFile::ENCRYPTION_1);
#endif

    std::set_new_handler(newHandler);

    // Set the diretory to look for all files
    SysPathName::rootEnvironmentVariable("MACH_ROOT");

    bool lobbyFlag = false;

    for (size_t i = 0; i < invokeArgs().size(); ++i)
    {
        const string& token = invokeArgs()[i];

        if (token == "-lobby")
            lobbyFlag = true;
        else if (token == "-?")
        {
            //          usage();
            //          return false;
        }
    }

    // Check other debugging environment variables
    bool preloadModels = getenv("CB_NOPRELOAD") == nullptr;
    bool preloadTextures = getenv("TEX_NOPRELOAD") == nullptr;
    bool preloadPlanets = getenv("CB_PLANETS_PRELOAD") != nullptr;

    // We must pick a co-orindate system: left-handed for Machines.
    MexCoordSystem::instance().set(MexCoordSystem::LEFT_HANDED);

    // This must be called before any render library object is created.  (It
    // ensures a correct destruction order for render library Singletons.)
    Ren::initialise();

    W4dRoot* root = pRoot_ = new W4dRoot(W4dRoot::W4dRootId());

    pDisplay_ = new RenDisplay(window());

    ErrorHandler::instance().pDisplay(pDisplay_);

    // Initially, pick the lowest-res 16-bit mode.
    pDisplay_->buildDisplayModesList();

    // Check for windowed mode
    if (!SysRegistry::instance().queryIntegerValue("Screen Resolution", "Windowed"))
        pDisplay_->useFullScreen();

    bool displayModeInitialized = false;
    if (SysRegistry::instance().queryIntegerValue(
            "Screen Resolution",
            "Lock Resolution",
            MachGuiStartupScreens::getDefaultLockScreenResolutionValue()))
    {
        int modeW = SysRegistry::instance().queryIntegerValue("Screen Resolution", "Width");
        int modeH = SysRegistry::instance().queryIntegerValue("Screen Resolution", "Height");
        int modeR = 0;

        if (modeW && modeH)
        {
            if (pDisplay_->fullScreen())
                modeR = SysRegistry::instance().queryIntegerValue("Screen Resolution", "Refresh Rate");

            const RenDisplay::Mode loadedMode = pDisplay_->findMode(modeW, modeH, modeR);
            if (loadedMode.isValid())
            {
                displayModeInitialized = pDisplay_->useMode(loadedMode);
            }

            if (!displayModeInitialized)
            {
                char aBuffer[128];
                snprintf(
                    &aBuffer[0],
                    sizeof(aBuffer),
                    "The preferred screen mode (%dx%d; %d Hz) is not available",
                    modeW,
                    modeH,
                    modeR);
                SysWindowsAPI::messageBox(aBuffer, name().c_str());
            }
        }
    }

    if (!displayModeInitialized)
    {
        const RenDisplay::Mode desktopMode = pDisplay_->getDesktopDisplayMode();
        displayModeInitialized = pDisplay_->useMode(desktopMode);
    }

    if (!displayModeInitialized)
    {
        const RenDisplay::Mode failSafeMode = pDisplay_->getFailSafeDisplayMode();
        displayModeInitialized = pDisplay_->useMode(failSafeMode);
        if (!displayModeInitialized)
        {
            char aBuffer[128];
            snprintf(
                &aBuffer[0],
                sizeof(aBuffer),
                "Failed to initialize the failsafe screen mode (%dx%d; %d Hz). Giving up.",
                failSafeMode.width(),
                failSafeMode.height(),
                failSafeMode.refreshRate());
            SysWindowsAPI::messageBox(aBuffer, name().c_str());
            return false;
        }
    }

    // tell the display what is the lowest resolution mode it is allowed to use
    pDisplay_->lowestAllowedMode(640, 480, 16);

    // if current mode uses too much memory, switch to a mode fitting in video memory
    //  bool modeSet = pDisplay_->setHighestAllowedMode();
    //  ASSERT(modeSet, "Could not find a mode fitting in video memory");

    // Ask the mouse to give us coordinates scale to the current Direct3D
    // resolution (which doesn't necessarilly match the Windows resolution).
    const RenDisplay::Mode& mode = pDisplay_->currentMode();

    {
        int scaleFactorPercents = SysRegistry::instance().queryIntegerValue("Options\\Scale Factor", "Value");
        if (scaleFactorPercents == 0)
        {
            if (mode.width() > 1024 && mode.height() > 768)
            {
                scaleFactorPercents = 200;
            }
            else
            {
                scaleFactorPercents = 100;
            }
        }
        else if (scaleFactorPercents > 100)
        {
            if (mode.width() < 1024 || mode.height() < 768)
            {
                spdlog::info("The scale factor from preferences does not fit in the window resolution");
                scaleFactorPercents = 100;
            }
        }

        spdlog::info("Using scale factor {}%", scaleFactorPercents);
        MachGui::setUiScaleFactor(scaleFactorPercents / 100.0);
    }

    DevMouse::instance().scaleCoordinates(mode.width(), mode.height());

    spdlog::info("Initializing the rendering device...");
    std::unique_ptr<RenDevice> pDevice = std::make_unique<RenDevice>(pDisplay_);
    if (!pDevice->initialize())
        return false;

    spdlog::info("Initializing SceneManager...");
    manager_ = new W4dSceneManager(std::move(pDevice), root);
    manager_->pDevice()->debugTextCoords(204, 0);
    manager_->useLevelOfDetail(
        !SysRegistry::instance().queryIntegerValue("Options\\Graphics Complexity\\LOD", "Value"));

    {
        int maxDomainDepth = 96; // The original game had 36
        const char maxDomainDepthKey[] = "Options\\Graphics Complexity\\Max Domain Depth";
        maxDomainDepth = SysRegistry::instance().queryIntegerValue(maxDomainDepthKey, "Value", maxDomainDepth);
        manager_->setMaxDomainDepth(maxDomainDepth);
        SysRegistry::instance().setIntegerValue(maxDomainDepthKey, "Value", maxDomainDepth);
    }

    // set this after the device has been created: we need the capability class
    // to find out how much memory is available for display
    //  bool highestModeSet = manager_->pDevice()->setHighestAllowedDisplayMode();
    //  ASSERT(highestModeSet, "Could tot find a mode fitting in the amount of display memory available");

    // Register the scene manager with the render libary manager
    HAL_STREAM("D3dApp::clientStartup sceneManager\n");
    W4dManager::instance().sceneManager(manager_);

    // Restore users gamma correction setting
    if (pDisplay_->supportsGammaCorrection())
    {
        SysRegistry::KeyHandle handle;
        if (SysRegistry::instance().onlyOpenKey("Options\\Gamma Correction", &handle) == SysRegistry::SUCCESS)
        {
            double gammaValue = SysRegistry::instance().queryIntegerValue("Options\\Gamma Correction", "Value");
            gammaValue /= GAMMA_REG_MULTIPLIER;
            gammaValue = mexClamp(gammaValue, GAMMA_LOWER_LIMIT, GAMMA_UPPER_LIMIT);
            pDisplay_->gammaCorrection(gammaValue);
        }
    }

    // Get top left offset for images
    int xOffset = (mode.width() - 640) / 2;
    int yOffset = (mode.height() - 480) / 2;

    // moved the sequence of calls slightly - the init sound call must still be done first though
    //     initSound();

    if (lobbyFlag)
    {
        // Display a loading screen.
        RenSurface frontBuffer = manager_->pDevice()->frontSurface();
        frontBuffer.filledRectangle(Ren::Rect(0, 0, mode.width(), mode.height()), RenColour::black());
        RenSurface waitBmp = RenSurface::createSharedSurface("gui/menu/waitlobb.bmp", frontBuffer);
        frontBuffer.simpleBlit(waitBmp, xOffset, yOffset);

        // Initialise lobby code
        NetNetwork::instance().instantiateLobby();
        MachLogNetwork::instance().launchFromLobbyInfo();

        // Display progress loading screen.
        // Call it twice to ensure on both buffers.
        RenSurface waitBmp2 = RenSurface::createSharedSurface("gui/menu/wait.bmp", frontBuffer);
        frontBuffer.simpleBlit(waitBmp2, xOffset, yOffset);
    }
    else
    {
        // Display a loading screen.
        RenSurface frontBuffer = manager_->pDevice()->frontSurface();
        frontBuffer.filledRectangle(Ren::Rect(0, 0, mode.width(), mode.height()), RenColour::black());
        RenSurface waitBmp = RenSurface::createSharedSurface("gui/menu/wait.bmp", frontBuffer);
        frontBuffer.simpleBlit(waitBmp, xOffset, yOffset);
        // Call it twice to draw on both front and back buffers
        manager_->pDevice()->display()->flipBuffers();
        frontBuffer.simpleBlit(waitBmp, xOffset, yOffset);
    }

    if (lobbyFlag)
    {
        // Display progress loading screen.
        RenSurface frontBuffer = manager_->pDevice()->frontSurface();
        frontBuffer.filledRectangle(Ren::Rect(0, 0, mode.width(), mode.height()), RenColour::black());
        RenSurface waitBmp = RenSurface::createSharedSurface("gui/menu/wait.bmp", frontBuffer);
        frontBuffer.simpleBlit(waitBmp, xOffset, yOffset);
        // Call it twice to draw on both front and back buffers
        manager_->pDevice()->display()->flipBuffers();
        frontBuffer.simpleBlit(waitBmp, xOffset, yOffset);
    }
    // Draw copyright note, store it in a way preventing from modification
    {
        const std::string note("NOT for sale, for testing purposes only.");
        RenSurface frontBuffer = manager_->pDevice()->frontSurface();
        frontBuffer.drawText(xOffset + 6, yOffset + 6, note, RenColour::yellow());
        // Call it twice to draw on both front and back buffers
        manager_->pDevice()->display()->flipBuffers();
        frontBuffer.drawText(xOffset + 6, yOffset + 6, note, RenColour::yellow());
    }

    ProgressIndicator progressIndicator(xOffset, yOffset);
    progressIndicator.setLimits(0.00, 0.18);

    // moved higher due to call sequence problems in lobbying when very slow machines host with very fast machines
    // in the game.
    StartedFromLobby startedType = NORMAL_START;
    if (lobbyFlag)
        startedType = LOBBY_START;
    spdlog::info("Initializing GUI...");
    initialiseGui(startedType, &progressIndicator);

    progressIndicator.report(100, 100);
    progressIndicator.setLimits(0.18, 0.20);

    initSound();

    // Start to play a CD to give the poor user something to listen to whilst
    // the models load...
    initMusic();

    progressIndicator.setLimits(0.20, 0.35);
    initProfiling(&progressIndicator);

    progressIndicator.report(100, 100);
    progressIndicator.setLimits(0.35, 0.37);
    pStartupScreens_->initializeVolumes();
    pStartupScreens_->initializeCursorOptions();

    MachPhysData::instance();

    MachPhysPreload::registerSounds();
    loadSounds();

    progressIndicator.report(100, 100);
    progressIndicator.setLimits(0.37, 0.40);

    // Set up the texture search path.
    RenTexManager::PathNames searchList = RenTexManager::instance().searchList();

    // We set different search paths depending on the amount
    // of video memory available for texture
    //  bool doLoad2MBytesTexture = !manager_->pDevice()->capabilities().supports4MBytesTextureSet();
    bool doLoad2MBytesTexture = false;
    // Check to see if texture4 directory even exists. The user may have done a minimum
    // install in which case texture4 directory will not be available
    if (! doLoad2MBytesTexture)
    {
        SysPathName texture4Dir("models/texture4");
        if (! texture4Dir.existsAsDirectory())
        {
            doLoad2MBytesTexture = true;
        }
    }

    progressIndicator.report(100, 100);
    progressIndicator.setLimits(0.40, 0.71);

    if (preloadTextures)
    {
        spdlog::info("Preloading model and weapons texture bitmaps...");
        if (doLoad2MBytesTexture)
        {
            pTextureSet_ = new RenTextureSet("models/texture2", &progressIndicator);
            // planet surface is going to use the search list as a set of flags
            // to decide 1) whether it should preload or not 2) whether it should
            // use texture2 or texture4... Isn't that pretty ?
            searchList.push_back(SysPathName("preload/texture2"));
            RenTexManager::instance().searchList(searchList);
        }
        else
        {
            pTextureSet_ = new RenTextureSet("models/texture4", &progressIndicator);
            // planet surface is going to use the search list as a set of flags
            // to decide 1) whether it should preload or not 2) whether it should
            // use texture2 or texture4... Isn't that pretty ?
            searchList.push_back(SysPathName("preload/texture4"));
            RenTexManager::instance().searchList(searchList);
        }
        LIONEL_STREAM("done preloading model and weapons texture bitmaps " << std::endl);
    }
    else
    {
        // set path search list for run-time loading of the textures
        LIONEL_STREAM("setting up search path for run-time models texture loading  " << std::endl);
        if (doLoad2MBytesTexture)
        {
            searchList.push_back(SysPathName("models/texture2"));
            searchList.push_back(SysPathName("models/texture2/exp"));
            searchList.push_back(SysPathName("models/texture2/fire"));
            searchList.push_back(SysPathName("models/texture2/smoke"));
            RenTexManager::instance().searchList(searchList);
        }
        else
        {
            searchList.push_back(SysPathName("models/texture4"));
            searchList.push_back(SysPathName("models/texture4/exp"));
            searchList.push_back(SysPathName("models/texture4/fire"));
            searchList.push_back(SysPathName("models/texture4/smoke"));
            RenTexManager::instance().searchList(searchList);
        }
        LIONEL_STREAM("searchList  " << searchList);
        LIONEL_STREAM("done setting up search path for run-time models texture loading  " << std::endl);
    }

    progressIndicator.setLimits(0.71, 0.96);

    // Preload the models if required
    if (preloadModels)
    {
        spdlog::info("Loading models...");
        if (MachPhysPreload::persistentFileName().existsAsFile())
        {
            /*if( not lobbyFlag )*/
            {
                DevTimer t1;
                DevTimer t2;
                MachPhysPreload::persistentPreload(MachPhysPreload::ECHO_PROGRESS, &progressIndicator);
                std::cout << "Time for persistent preload " << t1.time() << std::endl;

                t1.time(0.0);
                MachPhysPreload::preloadWeaponsAndEffects(MachPhysPreload::ECHO_PROGRESS);
                MachPhysPreload::extrasPreload(MachPhysPreload::ECHO_PROGRESS);
                std::cout << "Time for post-persistent preload " << t1.time() << std::endl;
                std::cout << "Time for total preload " << t2.time() << std::endl;
            }
            //          else
            //          {
            //              //register MDCs.
            //              MachPhysPersistence::instance();
            //          }
        }
        else
        {
            DevTimer t1;
            MachPhysPreload::swPreload(MachPhysPreload::ECHO_PROGRESS);
            std::cout << "Time for total preload " << t1.time() << std::endl;
            t1.time(0.0);
            MachPhysPreload::persistentSave(MachPhysPreload::ECHO_PROGRESS);
            std::cout << "Time for persistent save " << t1.time() << std::endl;
        }
    }
    else
    {
        // if no preload then call the instance to ensure that sub classes have all been registered.
        MachPhysPersistence::instance();
    }

    if (preloadPlanets)
        loadPlanets();

    // After loading, dump out the list of all the textures.
    // This helps determine exactly how much texture memory is used where.
    RENDER_STREAM(RenTexManager::instance());

    manager_->hideStats();

    W4dShadowProjected2d::shadowPlaneZ(0.025);

    HAL_STREAM("D3dApp::clientStartup checkSuspend SimManager\n");

    checkSuspendSimManager();

    progressIndicator.setLimits(0.0, 1.0);
    progressIndicator.report(100, 100); // 100& done

    initialised_ = true;
    finishTimer_.time(0);

    HAL_STREAM("D3dApp::clientStartup simResetTimer\n");
    simResetTime();

    spdlog::info("The client startup is done.");

    return true;
}

void SDLApp::clientShutdown()
{
    // Move the window completely off-screen before reseting from fullscreen
    // mode.  Smacker appears to use the window as temporary space for
    // decompression.  Thus when we exit, it is partly covered in garbage.
    // This is the easiest way to hide it.  We can't permanently hide it
    // during normal execution because it screws up the mouse co-ordinates./
    //  moveWindow(10000, 10000);

    cleanUpGui();

    DevMouse::instance().unhide();
    delete pRoot_;
    delete manager_;
    // Tell W4dManager about destruction of sceneManager
    W4dManager::instance().clearSceneManager();
    delete pDisplay_;
    delete pTextureSet_;
    cleanUpSound();
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

void SDLApp::initProfiling(IProgressReporter* pReporter)
{
    spdlog::info("Initializing profiler...");

    double profileInterval = 50.0;
    char* pMs = getenv("PROFILE_RATE");
    if (pMs)
        profileInterval = atof(pMs);

    ProProfiler::instance(pReporter);
    ProProfiler::instance().traceInterval(profileInterval / 1000.0);

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
}

// virtual
bool SDLApp::activate()
{
    if (!manager_)
        return false;

    if (manager_->pDevice())
        manager_->pDevice()->activate();

    activateGui();

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
        const string flag = invokeArgs()[0];
        if (flag == "-w" && invokeArgs().size() == 3)
        {
            winWidth_ = atoi(invokeArgs()[1].c_str());
            winHeight_ = atoi(invokeArgs()[2].c_str());
        }
    }

    w = winWidth_;
    h = winHeight_;
}

/* End D3DAPP.CPP ***************************************************/
