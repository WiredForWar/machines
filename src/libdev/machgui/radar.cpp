/*
 * R A D A R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/radar.hpp"
#include "gui/gui.hpp"
#include "machgui/gui.hpp"
#include "machgui/actnamid.hpp"
#include "machgui/actbmpnm.hpp"
#include "gui/painter.hpp"
#include "machphys/machphys.hpp"
#include "machphys/objdata.hpp"
#include "machphys/machdata.hpp"
#include "machlog/actor.hpp"
#include "machlog/canattac.hpp"
#include "machlog/machine.hpp"
#include "machlog/races.hpp"
#include "machlog/p1mchndl.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/eulerang.hpp"
#include "system/pathname.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "render/device.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "device/time.hpp"

#define RADAR_ANIMATION_FRAMES 10
#define RADAR_ANIMATION_TIME 0.5

class MachGuiRadarImpl
{
public:
    MachGuiRadarImpl();

    MachActor* pActor_ {};
    GuiBitmap healthBmp_[3];
    GuiBitmap armourBmp_[3];
    GuiBitmap radarBackdropBmp_;
    GuiBitmap radarDomeBmp_;
    MachLog1stPersonHandler* pLogHandler_ {};
    bool justEnteredFirstPerson_;
    GuiBitmap radarStartupFrames_[RADAR_ANIMATION_FRAMES];
    GuiBitmap machineIcon_;
    bool hpAboveCritical_ {};
    double animationEndTime_;
    int frameNumber_;
};

MachGuiRadarImpl::MachGuiRadarImpl()
    : radarBackdropBmp_(RenSurface::createSharedSurface(
        Gui::getScaledImagePath("gui/fstpersn/radar/rmmap.bmp"),
        Gui::backBuffer()))
    , radarDomeBmp_(RenSurface::createSharedSurface(
          Gui::getScaledImagePath("gui/fstpersn/radar/dome.bmp"),
          Gui::backBuffer()))
{
    radarBackdropBmp_.enableColourKeying();
    radarDomeBmp_.enableColourKeying();
}

MachGuiRadar::MachGuiRadar(GuiDisplayable* pParent, const Gui::Coord& relPos)
    : GuiDisplayable(pParent, Gui::Box(relPos, 1, 1))
{
    pImpl_ = new MachGuiRadarImpl;

    // Switch on colour keying for radar images
    for (MachPhys::Race race : MachPhys::AllRaces)
    {
        machineImage()[race].enableColourKeying();
        podImage()[race].enableColourKeying();
        constructionImage()[race].enableColourKeying();
        missileEmplacementImage()[race].enableColourKeying();
    }
    oreImage().enableColourKeying();
    debrisImage().enableColourKeying();
    artefactImage().enableColourKeying();
    for (int loop = 0; loop < 16; ++loop)
    {
        arrowImage()[loop].enableColourKeying();
    }

    TEST_INVARIANT;
}

MachGuiRadar::~MachGuiRadar()
{
    TEST_INVARIANT;
    delete pImpl_;
}

// virtual
void MachGuiRadar::doDisplay()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(bool, justEnteredFirstPerson_);
    CB_DEPIMPL(GuiBitmap, machineIcon_);
    CB_DEPIMPL(double, animationEndTime_);
    CB_DEPIMPL(int, frameNumber_);

    double now = DevTime::instance().time();

    if (justEnteredFirstPerson_ == true)
    {
        animationEndTime_ = now + RADAR_ANIMATION_TIME;
        justEnteredFirstPerson_ = false;
    }

    if (animationEndTime_ > now || frameNumber_ < RADAR_ANIMATION_FRAMES)
    {
        displayAnimatedRadarFrame();
    }
    else
    {
        displayRadarBlips();
        displayHealthArmour();

        // Display machine icon
        GuiPainter::instance().blit(
            machineIcon_,
            absoluteBoundary().topLeft() + Gui::Coord(180, 30) * Gui::uiScaleFactor());
    }

    ++frameNumber_;
}

void MachGuiRadar::displayHealthArmour()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(MachActor*, pActor_);
    CB_DEPIMPL_ARRAY(GuiBitmap, healthBmp_);
    CB_DEPIMPL_ARRAY(GuiBitmap, armourBmp_);
    CB_DEPIMPL(bool, hpAboveCritical_);

    if (pActor_)
    {
        const double healthHeight = healthBmp_[0].height();
        const double armourHeight = armourBmp_[0].height();

        float hpRatio = pActor_->hpRatio();
        float apRatio = pActor_->armourRatio();

        double displayHealthHeight = hpRatio * healthHeight;
        double displayArmourHeight = apRatio * armourHeight;

        const int minX = absoluteBoundary().minCorner().x();
        const int minY = absoluteBoundary().minCorner().y();

        const float LOW_THRESHOLD = 1.0 / 3.0;
        const float MID_THRESHOLD = 1.7 / 3.0;
        int hpBmpIndex = 0;
        int apBmpIndex = 0;

        bool newHpAboveCritical = hpRatio > 0.2;

        // Play sound if HP are at critical level ( below 20% )
        if (!newHpAboveCritical && hpAboveCritical_)
            MachGuiSoundManager::instance().playSound("gui/sounds/hpcritic.wav");

        hpAboveCritical_ = newHpAboveCritical;

        // Work out which HP bitmap to display
        if (hpRatio <= LOW_THRESHOLD)
            hpBmpIndex = 2;
        else if (hpRatio <= MID_THRESHOLD)
            hpBmpIndex = 1;

        // Work out which AP bitmap to display
        if (apRatio <= LOW_THRESHOLD)
            apBmpIndex = 2;
        else if (apRatio <= MID_THRESHOLD)
            apBmpIndex = 1;

        const int healthBarXOffset = 216 * Gui::uiScaleFactor();
        const int armourBarXOffset = 234 * Gui::uiScaleFactor();
        const int barYOffset = 111 * Gui::uiScaleFactor();

        GuiPainter::instance().blit(
            healthBmp_[hpBmpIndex],
            Gui::Box(0, healthHeight - displayHealthHeight, healthBmp_[hpBmpIndex].width(), healthHeight),
            Gui::Coord(minX + healthBarXOffset, minY + barYOffset + healthHeight - displayHealthHeight));
        GuiPainter::instance().blit(
            armourBmp_[apBmpIndex],
            Gui::Box(0, armourHeight - displayArmourHeight, armourBmp_[apBmpIndex].width(), armourHeight),
            Gui::Coord(minX + armourBarXOffset, minY + barYOffset + armourHeight - displayArmourHeight));
    }
}

void MachGuiRadar::displayRadarBlips()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(GuiBitmap, radarBackdropBmp_);
    CB_DEPIMPL(GuiBitmap, radarDomeBmp_);
    CB_DEPIMPL(MachActor*, pActor_);
    CB_DEPIMPL(MachLog1stPersonHandler*, pLogHandler_);

    // Blit see through portion of map
    GuiPainter::instance().blit(radarBackdropBmp_, absoluteBoundary().minCorner());

    // Display arrow in centre of radar.
    displayMotionDirection();

    // Blit icons onto radar
    if (pActor_)
    {
        const int screenScannerRadius = 55 * Gui::uiScaleFactor();
        const Gui::Coord screenScannerCentre = Gui::Coord(102, 103) * Gui::uiScaleFactor();
        double scannerScaler = 1.0;
        double scannerRange = 1.0;

        MexTransform3d fstPersonActorInverseTrans = pActor_->globalTransform();
        MexPoint2d fstPersonPos = fstPersonActorInverseTrans.position();
        MexEulerAngles fstPersonAngles = fstPersonActorInverseTrans.rotationAsEulerAngles();
        fstPersonAngles.azimuth(fstPersonAngles.azimuth() + MexRadians(Mathex::PI_DIV_2));

        // Add head turn ( if machine can turn head! )
        if (pLogHandler_ && pLogHandler_->canTurnHead())
        {
            fstPersonAngles.azimuth(fstPersonAngles.azimuth() + pLogHandler_->currentHeadAngle());
        }

        fstPersonActorInverseTrans.rotation(fstPersonAngles);

        fstPersonActorInverseTrans.invert();
        if (pActor_->objectIsMachine())
        {
            scannerRange = pActor_->asMachine().machineData().scannerRange();
            scannerScaler = screenScannerRadius / scannerRange;
        }

        double sqrScannerRange = scannerRange * scannerRange;

        const MachLogRaces& races = MachLogRaces::instance();
        const MachLogRaces::Objects& allObjects = races.objects();

        for (const MachActor* pScannedActor : allObjects)
        {
            if (pScannedActor == pActor_)
                continue;

            MexPoint3d pos = pScannedActor->globalTransform().position();
            const MexPoint2d pos2d = pos;

            if (pos2d.sqrEuclidianDistance(fstPersonPos) >= sqrScannerRange)
                continue;

            // Get info about scanned actors race
            MachPhys::Race scannedActorRace = pScannedActor->displayMapAndIconRace();

            // Convert pos relative to fstPerson machine
            fstPersonActorInverseTrans.transform(&pos);

            const MexPoint2d scaledPos2d = MexPoint2d(pos) * scannerScaler;
            const Gui::Coord indicatorPos = absoluteBoundary().topLeft() + screenScannerCentre + scaledPos2d;

            // Blit blob
            if (pScannedActor->objectIsMachine() && !pScannedActor->asMachine().insideBuilding()
                && // Ignore any machines inside buildings
                !pScannedActor->asMachine().insideAPC()) // Ignore any machines inside APCs
            {
                GuiPainter::instance().blit(machineImage()[scannedActorRace], indicatorPos);
            }
            else if (pScannedActor->objectIsConstruction())
            {
                if (pScannedActor->objectType() == MachLog::POD)
                {
                    GuiPainter::instance().blit(podImage()[scannedActorRace], indicatorPos);
                }
                else if (pScannedActor->objectType() == MachLog::MISSILE_EMPLACEMENT)
                {
                    GuiPainter::instance().blit(missileEmplacementImage()[scannedActorRace], indicatorPos);
                }
                else
                {
                    GuiPainter::instance().blit(constructionImage()[scannedActorRace], indicatorPos);
                }
            }
            else if (pScannedActor->objectIsDebris())
            {
                GuiPainter::instance().blit(debrisImage(), indicatorPos);
            }
            else if (pScannedActor->objectIsOreHolograph())
            {
                GuiPainter::instance().blit(oreImage(), indicatorPos);
            }
            else if (pScannedActor->objectIsArtefact())
            {
                GuiPainter::instance().blit(artefactImage(), indicatorPos);
            }
        }

        // Blit middle bit of radar
        const Gui::Coord domeCoord = Gui::Coord(100, 101) * Gui::uiScaleFactor();
        GuiPainter::instance().blit(radarDomeBmp_, absoluteBoundary().topLeft() + domeCoord);
    }
}

void MachGuiRadar::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiRadar& t)
{

    o << "MachGuiRadar " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiRadar " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachGuiRadar::actor(MachActor* pActor)
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(MachActor*, pActor_);
    CB_DEPIMPL(GuiBitmap, machineIcon_);
    CB_DEPIMPL(bool, hpAboveCritical_);

    pActor_ = pActor;

    // Get icon to display on radar
    MachPhys::WeaponCombo wc = MachPhys::N_WEAPON_COMBOS;
    if (pActor->objectIsCanAttack())
    {
        wc = pActor->asCanAttack().weaponCombo();
    }

    machineIcon_ = MachGui::getScaledImage(MachActorBitmaps::name(
        pActor_->objectType(),
        pActor_->subType(),
        pActor_->asMachine().hwLevel(),
        wc,
        pActor_->race(),
        true));

    // When HP fall below critical level trigger sound
    hpAboveCritical_ = false;
}

void MachGuiRadar::resetActor()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(MachActor*, pActor_);

    pActor_ = nullptr;
}

// static
GuiBitmap* MachGuiRadar::machineImage()
{
    static GuiBitmap machinePixel[MachPhys::N_RACES] = {
        Gui::getScaledImage("gui/fstpersn/radar/blips/rmachpix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/bmachpix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/gmachpix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/ymachpix.bmp"),
    };

    return machinePixel;
}

// static
GuiBitmap* MachGuiRadar::constructionImage()
{
    static GuiBitmap constructionPixel[MachPhys::N_RACES] = {
        Gui::getScaledImage("gui/fstpersn/radar/blips/rconspix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/bconspix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/gconspix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/yconspix.bmp"),
    };

    return constructionPixel;
}

// static
GuiBitmap* MachGuiRadar::podImage()
{
    static GuiBitmap podPixel[MachPhys::N_RACES] = {
        Gui::getScaledImage("gui/fstpersn/radar/blips/rpodpix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/bpodpix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/gpodpix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/ypodpix.bmp"),
    };

    return podPixel;
}

// static
GuiBitmap* MachGuiRadar::missileEmplacementImage()
{
    static GuiBitmap mePixel[MachPhys::N_RACES] = {
        Gui::getScaledImage("gui/fstpersn/radar/blips/rmisspix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/bmisspix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/gmisspix.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/blips/ymisspix.bmp"),
    };

    return mePixel;
}

// static
GuiBitmap* MachGuiRadar::arrowImage()
{
    static GuiBitmap arrowImages[16] = {
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point1.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point2.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point3.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point4.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point5.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point6.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point7.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point8.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point9.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point10.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point11.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point12.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point13.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point14.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point15.bmp"),
        Gui::getScaledImage("gui/fstpersn/radar/arrow/point16.bmp"),
    };

    return arrowImages;
}

// static
GuiBitmap& MachGuiRadar::debrisImage()
{
    static GuiBitmap debrisPixel = Gui::getScaledImage("gui/fstpersn/radar/blips/debris.bmp");
    return debrisPixel;
}

// static
GuiBitmap& MachGuiRadar::artefactImage()
{
    static GuiBitmap artifactPixel = Gui::getScaledImage("gui/fstpersn/radar/blips/artefact.bmp");
    return artifactPixel;
}

// static
GuiBitmap& MachGuiRadar::oreImage()
{
    static GuiBitmap orePixel = Gui::getScaledImage("gui/fstpersn/radar/blips/ore.bmp");
    return orePixel;
}

void MachGuiRadar::logHandler(MachLog1stPersonHandler* pLogHandler)
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(MachLog1stPersonHandler*, pLogHandler_);

    pLogHandler_ = pLogHandler;
}

void MachGuiRadar::resetLogHandler()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(MachLog1stPersonHandler*, pLogHandler_);

    pLogHandler_ = nullptr;
}

void MachGuiRadar::displayMotionDirection()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(MachLog1stPersonHandler*, pLogHandler_);

    int index = 0;
    if (pLogHandler_ && pLogHandler_->canTurnHead())
    {
        MexDegrees headAngle = pLogHandler_->currentHeadAngle();
        MATHEX_SCALAR headAngleScalar = -headAngle.asScalar();

        // Force degrees to lie between 0 - 360.
        while (headAngleScalar < 0.0)
            headAngleScalar += 360.0;

        while (headAngleScalar >= 360.0)
            headAngleScalar -= 360.0;

        headAngleScalar = headAngleScalar / 360.0;
        index = headAngleScalar * 16;
    }

    const Gui::Coord arrowCoord = Gui::Coord(85, 85) * Gui::uiScaleFactor();
    GuiPainter::instance().blit(arrowImage()[index], absoluteBoundary().topLeft() + arrowCoord);
}

void MachGuiRadar::displayAnimatedRadarFrame()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(double, animationEndTime_);
    CB_DEPIMPL_ARRAY(GuiBitmap, radarStartupFrames_);
    CB_DEPIMPL(int, frameNumber_)

    double timeDiff = animationEndTime_ - DevTime::instance().time();
    int frame = timeDiff * (RADAR_ANIMATION_FRAMES / RADAR_ANIMATION_TIME);

    if (frame >= RADAR_ANIMATION_FRAMES)
    {
        frame = RADAR_ANIMATION_FRAMES - 1;
    }

    frame = (RADAR_ANIMATION_FRAMES - 1) - frame;

    frame = std::min(frameNumber_, frame);

    RenDevice& device = *W4dManager::instance().sceneManager()->pDevice();
    const int w = device.windowWidth();

    const auto &frameTexture = radarStartupFrames_[frame];
    int frameWidth = frameTexture.requestedSize().isNull() ? frameTexture.width() : frameTexture.requestedSize().width;

    // Blit to screen.
    GuiPainter::instance().blitInRequestedSize(frameTexture, Gui::Coord(w - frameWidth, absoluteBoundary().top()));
}

void MachGuiRadar::initialise()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(bool, justEnteredFirstPerson_);
    CB_DEPIMPL(int, frameNumber_);

    justEnteredFirstPerson_ = true;
    frameNumber_ = 0;
}

void MachGuiRadar::loadBitmaps()
{
    CB_DEPIMPL_ARRAY(GuiBitmap, radarStartupFrames_);
    CB_DEPIMPL_ARRAY(GuiBitmap, healthBmp_);
    CB_DEPIMPL_ARRAY(GuiBitmap, armourBmp_);

    for (int loop = 0; loop < RADAR_ANIMATION_FRAMES; ++loop)
    {
        std::string framePath("gui/fstpersn/radar/radar");
        char buffer[3];

        // framePath += itoa( loop, buffer, 10 );
        sprintf(buffer, "%d", loop);
        framePath += buffer;

        radarStartupFrames_[loop] = Gui::requestScaledImage(framePath);
        radarStartupFrames_[loop].enableColourKeying();
    }

    // Load health and armour bars
    healthBmp_[0] = Gui::getScaledImage("gui/fstpersn/radar/rhealth.bmp");
    healthBmp_[1] = Gui::getScaledImage("gui/fstpersn/radar/rhealth2.bmp");
    healthBmp_[2] = Gui::getScaledImage("gui/fstpersn/radar/rhealth3.bmp");
    armourBmp_[0] = Gui::getScaledImage("gui/fstpersn/radar/rarmour.bmp");
    armourBmp_[1] = Gui::getScaledImage("gui/fstpersn/radar/rarmour2.bmp");
    armourBmp_[2] = Gui::getScaledImage("gui/fstpersn/radar/rarmour3.bmp");
}

void MachGuiRadar::unloadBitmaps()
{
    CB_DEPIMPL_ARRAY(GuiBitmap, radarStartupFrames_);
    CB_DEPIMPL_ARRAY(GuiBitmap, healthBmp_);
    CB_DEPIMPL_ARRAY(GuiBitmap, armourBmp_);

    for (int loop = 0; loop < RADAR_ANIMATION_FRAMES; ++loop)
    {
        radarStartupFrames_[loop] = GuiBitmap();
    }

    // Reset health and armour bars
    healthBmp_[0] = GuiBitmap();
    healthBmp_[1] = GuiBitmap();
    healthBmp_[2] = GuiBitmap();
    armourBmp_[0] = GuiBitmap();
    armourBmp_[1] = GuiBitmap();
    armourBmp_[2] = GuiBitmap();
}

/* End RADAR.CPP ****************************************************/
