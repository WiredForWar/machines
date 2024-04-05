/*
 * G U I . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */
// #include "windows.h"
#include "stdlib/string.hpp"
#include "system/pathname.hpp"
#include "network/netnet.hpp"

#include "machgui/gui.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/painter.hpp"
#include "gui/gui.hpp"
#include "gui/restring.hpp"

#include "system/pathname.hpp"
#include "machgui/actbmpnm.hpp"
#include "machlog/canattac.hpp"
#include "machlog/machine.hpp"
#include <cassert>

static GuiBitmap& privateLongGlowBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateLongYellowGlowBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateButtonGlowBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateButtonDisableBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateMenuScrollUpBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateMenuScrollDownBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateOkMsgBoxBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateOkCancelMsgBoxBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateTickBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateDarkTickBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateLargeTickBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateLargeDarkTickBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateDropDownBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateHostBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateSlideBtnBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateControlPanelBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

static GuiBitmap& privateInTransitBmp()
{
    static GuiBitmap bitmap;
    return bitmap;
}

// static
GuiBitmap MachGui::createIconForMachine(MachLogMachine* machine, bool firstPersonIcon)
{
    // This is needed for the bitmap name function. )^:?
    MachPhys::WeaponCombo wc = MachPhys::N_WEAPON_COMBOS;
    if (machine->objectIsCanAttack())
    {
        wc = machine->asCanAttack().weaponCombo();
    }

    GuiBitmap icon = MachGui::getScaledImage(MachActorBitmaps::name(
        machine->objectType(),
        machine->subType(),
        machine->asMachine().hwLevel(),
        wc,
        machine->race(),
        firstPersonIcon));
    return icon;
}

// static
const Gui::Colour& MachGui::mVIIIBackgroundColour()
{
    static Gui::Colour result_(35.0 / 255.0, 30.0 / 255.0, 61.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::OFFWHITE()
{
    static Gui::Colour result_(239.0 / 255.0, 238.0 / 255.0, 222.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::VERYDARKGREY()
{
    static Gui::Colour result_(20.0 / 255.0, 20.0 / 255.0, 20.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::ALMOSTBLACK()
{
    static Gui::Colour result_(8.0 / 255.0, 8.0 / 255.0, 8.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::VERYDARKPURPLE()
{
    static Gui::Colour result_(57.0 / 255.0, 49.0 / 255.0, 77.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::LIGHTPURPLE()
{
    static Gui::Colour result_(157.0 / 255.0, 156.0 / 255.0, 182.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::PURPLE()
{
    static Gui::Colour result_(78.0 / 255.0, 86.0 / 255.0, 113.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::PROGRESSGREEN()
{
    static Gui::Colour result_(0, 185.0 / 255.0, 0);
    return result_;
}

// static
const Gui::Colour& MachGui::MENUDARKGREEN()
{
    static Gui::Colour result_(1.0 / 255.0, 35.0 / 255.0, 1.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::SANDY()
{
    static Gui::Colour result_(226.0 / 255.0, 232.0 / 255.0, 157.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::DARKSANDY()
{
    static Gui::Colour result_(171.0 / 255.0, 178.0 / 255.0, 93.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::DARKRED()
{
    static Gui::Colour result_(120.0 / 255.0, 0, 0);
    return result_;
}

// static
const Gui::Colour& MachGui::DARKYELLOW()
{
    static Gui::Colour result_(120.0 / 255.0, 120.0 / 255.0, 0);
    return result_;
}

// static
const Gui::Colour& MachGui::DARKGREEN()
{
    static Gui::Colour result_(0, 120.0 / 255.0, 0);
    return result_;
}

// static
const Gui::Colour& MachGui::DARKBLUE()
{
    static Gui::Colour result_(0, 0, 120.0 / 255.0);
    return result_;
}

// static
const Gui::Colour& MachGui::DROPDOWNDARKGREEN()
{
    static Gui::Colour result_(7.0 / 255.0, 32.0 / 255.0, 0);
    return result_;
}

// static
const Gui::Colour& MachGui::DROPDOWNLIGHTGREEN()
{
    static Gui::Colour result_(32.0 / 255.0, 255.0 / 255.0, 15.0 / 255.0);
    return result_;
}

// static
void MachGui::drawNumber(GuiBitmap* pNumberArray, size_t value, Gui::Coord* pAbsCoord)
{
    do
    {
        size_t numright = value % 10;

        value /= 10;

        pAbsCoord->x(pAbsCoord->x() - pNumberArray[numright].width());

        GuiPainter::instance().blit(pNumberArray[numright], *pAbsCoord);
    } while (value);
}

// static
void MachGui::releaseMenuBmpMemory()
{
    GuiBitmap* allBitmaps[] = {
        &privateLongGlowBmp(),
        &privateLongYellowGlowBmp(),
        &privateButtonGlowBmp(),
        &privateButtonDisableBmp(),
        &privateMenuScrollUpBmp(),
        &privateMenuScrollDownBmp(),
        &privateOkMsgBoxBmp(),
        &privateOkCancelMsgBoxBmp(),
        &privateTickBmp(),
        &privateDarkTickBmp(),
        &privateLargeTickBmp(),
        &privateLargeDarkTickBmp(),
        &privateDropDownBmp(),
        &privateHostBmp(),
        &privateSlideBtnBmp(),
    };

    for (GuiBitmap* bitmap : allBitmaps)
    {
        bitmap->reset();
    }
}

// static
GuiBitmap& MachGui::longGlowBmp()
{
    GuiBitmap& pBmp = privateLongGlowBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/longglow.bmp");
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::longYellowGlowBmp()
{
    GuiBitmap& pBmp = privateLongYellowGlowBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/lonyglow.bmp");
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::buttonGlowBmp()
{
    GuiBitmap& pBmp = privateButtonGlowBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/glow.bmp");
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::buttonDisableBmp()
{
    GuiBitmap& pBmp = privateButtonDisableBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/disable.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::menuScrollUpBmp()
{
    GuiBitmap& pBmp = privateMenuScrollUpBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/scrollup.bmp");
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::menuScrollDownBmp()
{
    GuiBitmap& pBmp = privateMenuScrollDownBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/scrolldn.bmp");
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::okMsgBoxBmp()
{
    GuiBitmap& pBmp = privateOkMsgBoxBmp();

    if (pBmp.isNull())
    {
        ASSERT_FILE_EXISTS("gui/menu/msgbox.bmp");

        pBmp = Gui::bitmap("gui/menu/msgbox.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::okCancelMsgBoxBmp()
{
    GuiBitmap& pBmp = privateOkCancelMsgBoxBmp();

    if (pBmp.isNull())
    {
        ASSERT_FILE_EXISTS("gui/menu/msgbox2.bmp");

        pBmp = Gui::bitmap("gui/menu/msgbox2.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::tickBmp()
{
    GuiBitmap& pBmp = privateTickBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/tick.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::darkTickBmp()
{
    GuiBitmap& pBmp = privateDarkTickBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/tickd.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::largeDarkTickBmp()
{
    GuiBitmap& pBmp = privateLargeDarkTickBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/tickld.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::largeTickBmp()
{
    GuiBitmap& pBmp = privateLargeTickBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/tickl.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::dropDownBmp()
{
    GuiBitmap& pBmp = privateDropDownBmp();

    if (pBmp.isNull())
    {
        pBmp = Gui::bitmap("gui/menu/dropdown.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::hostBmp()
{
    GuiBitmap& pBmp = privateHostBmp();

    if (pBmp.isNull())
    {
        ASSERT_FILE_EXISTS("gui/menu/host.bmp");

        pBmp = Gui::bitmap("gui/menu/host.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::slideBtnBmp()
{
    GuiBitmap& pBmp = privateSlideBtnBmp();

    if (pBmp.isNull())
    {
        ASSERT_FILE_EXISTS("gui/menu/slidebtn.bmp");

        pBmp = Gui::bitmap("gui/menu/slidebtn.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::controlPanelBmp()
{
    GuiBitmap& pBmp = privateControlPanelBmp();

    if (pBmp.isNull())
    {
        ASSERT_FILE_EXISTS("gui/misc/backtile.bmp");

        pBmp = MachGui::getScaledImage("gui/misc/backtile");
    }

    return pBmp;
}

// static
GuiBitmap& MachGui::inTransitBmp()
{
    GuiBitmap& pBmp = privateInTransitBmp();

    if (pBmp.isNull())
    {
        ASSERT_FILE_EXISTS("gui/misc/transit.bmp");

        pBmp = Gui::bitmap("gui/misc/transit.bmp");
        pBmp.enableColourKeying();
    }

    return pBmp;
}

// static
const string& MachGui::closedText()
{
    static string text;
    if (text.length() == 0)
    {
        GuiResourceString str(IDS_MENU_CLOSED);
        text = str.asString();
    }

    return text;
}

// static
const string& MachGui::openText()
{
    static string text;
    if (text.length() == 0)
    {
        GuiResourceString str(IDS_MENU_OPEN);
        text = str.asString();
    }

    return text;
}

// static
const string& MachGui::computerText()
{
    static string text;
    if (text.length() == 0)
    {
        GuiResourceString str(IDS_MENU_COMPUTER);
        text = str.asString();
    }

    return text;
}

// static
const string& MachGui::unavailableText()
{
    static string text;
    if (text.length() == 0)
    {
        GuiResourceString str(IDS_MENU_UNAVAILABLE);
        text = str.asString();
    }

    return text;
}

// static
static double s_uiScaleFactor = 0;

MATHEX_SCALAR MachGui::uiScaleFactor()
{
    assert(s_uiScaleFactor > 0);
    return s_uiScaleFactor;
}

MATHEX_SCALAR MachGui::menuScaleFactor()
{
    return uiScaleFactor();
}

void MachGui::setUiScaleFactor(MATHEX_SCALAR scale)
{
    s_uiScaleFactor = scale;

    Menu::updateFonts();
}

// static
std::string MachGui::getScaledImagePath(std::string path)
{
    const bool hasBmpExtention = path.size() > 4 && path.substr(path.size() - 4, 4) == ".bmp";
    const MATHEX_SCALAR factor = uiScaleFactor();
    if (factor == 1)
    {
        if (hasBmpExtention)
            return path;

        return path + ".bmp";
    }

    // TODO: Fix later :eyes:
    if (hasBmpExtention)
    {
        const auto from = path.end() - 4;
        path.replace(from, path.end(), "_2x.png");
        return path;
    }

    return path + "_2x.png";
}

GuiBitmap MachGui::getScaledImage(std::string path)
{
    GuiBitmap image = Gui::requestScaledImage(path, uiScaleFactor());
    if (image.requestedSize().isNull())
        return image;

    RenSurface scaledSurface = RenSurface::createAnonymousSurface(image.requestedSize(), image);

    // Workaround artefacts in transparent pixels:
    scaledSurface.filledRectangle(image.requestedSize(), Gui::MAGENTA());
    scaledSurface.enableColourKeying();

    scaledSurface.stretchBlit(image);

    return scaledSurface;
}

float MachGui::getPhysMarkerLineWidth()
{
    const float scale = uiScaleFactor();

    if (scale >= 3.0f)
    {
        return 2.0f;
    }
    else if (scale > 1.0f)
    {
        return 1.5f;
    }

    return 1.0f;
}

// static
int MachGui::controlPanelInXPos()
{
    return 6 * uiScaleFactor();
}

// static
int MachGui::controlPanelOutXPos()
{
    return 169 * uiScaleFactor();
}

// static
int MachGui::controlPanelSlideInSpeed()
{
    return 15;
}

// static
int MachGui::controlPanelSlideOutSpeed()
{
    return 20;
}

// static
int MachGui::promptTextXOffset()
{
    return -7 * MachGui::uiScaleFactor();
}

// static
int MachGui::promptTextYOffset()
{
    return -24 * uiScaleFactor();
}

// static
int MachGui::mapAreaHeight()
{
    return 129 * uiScaleFactor();
}

int MachGui::barBorderThickness()
{
    return 1 * MachGui::uiScaleFactor();
}

int MachGui::barShadowThickness()
{
    return 1 * MachGui::uiScaleFactor();
}

int MachGui::barDividerThickness()
{
    return 1 * MachGui::uiScaleFactor();
}

int MachGui::barValueLineOffset()
{
    return 1 * MachGui::uiScaleFactor();
}

int MachGui::barValueLineThickness()
{
    // The '+' below is intentional:
    // We want 2px for 1X and 3px for 2X scales.
    return 1 + MachGui::uiScaleFactor();
}

int MachGui::iconIndexYOffset()
{
    // We want 0px for 1X and 1px for 2X scales.
    return MachGui::uiScaleFactor() - 1;
}

int MachGui::corralInfoTextSize()
{
    return MachGui::uiScaleFactor() == 1 ? 12 : 20;
}

int MachGui::navigationButtonNumberSize()
{
    return MachGui::uiScaleFactor() == 1 ? 12 : 20;
}

// static
double MachGui::doubleClickInterval()
{
    return 0.250; // 250 ms
}

// static
int MachGui::convertNetworkError(int errorCode)
{
    switch ((NetNetwork::NetNetworkStatus)errorCode)
    {
        case NetNetwork::NETNET_OK:
            return IDS_MENUMSG_NETSUCCESS;
        case NetNetwork::NETNET_CONNECTIONERROR:
            return IDS_MENUMSG_NETCONNECTIONERROR;
        case NetNetwork::NETNET_INVALIDCONNECTIONDATA:
            return IDS_MENUMSG_NETINVALIDCONNECTIONDATA;
        case NetNetwork::NETNET_INVALIDDATA:
            return IDS_MENUMSG_NETINVALIDDATA;
        case NetNetwork::NETNET_UNDEFINEDERROR:
            return IDS_MENUMSG_NETUNDEFINEDERROR;
        case NetNetwork::NETNET_CANTINITIALIZEDIRECTPLAY:
            return IDS_MENUMSG_NETCANTINITIALIZEDIRECTPLAY;
        case NetNetwork::NETNET_MEMORYERROR:
            return IDS_MENUMSG_NETMEMORYERROR;
        case NetNetwork::NETNET_SESSIONERROR:
            return IDS_MENUMSG_NETSESSIONERROR;
        case NetNetwork::NETNET_NODEERROR:
            return IDS_MENUMSG_NETNODEERROR;
    }
    return IDS_MENUMSG_NETSUCCESS;
}

// static
void MachGui::releaseInGameBmpMemory()
{
    GuiBitmap* allBitmaps[] = {
        &privateInTransitBmp(),
        &privateControlPanelBmp(),
    };

    for (GuiBitmap* bitmap : allBitmaps)
    {
        bitmap->reset();
    }
}

// static
bool MachGui::getCDRomDriveContainingMachinesCD(string& cdRomDrive, int CDNumber)
{
    PRE(CDNumber > 0 && CDNumber < 3);

    /*  if ( CDNumber == 1 )
        return getCDRomDriveContainingFile( cdRomDrive, "setupx.bin" );
    else
        return getCDRomDriveContainingFile( cdRomDrive, "setupy.bin" );*/
    return CDNumber == 2;
}

// static
bool MachGui::machinesCDIsAvailable(int CDNumber)
{
    PRE(CDNumber > 0 && CDNumber < 3);

    string cdRomDrive;

    return getCDRomDriveContainingMachinesCD(cdRomDrive, CDNumber);
}

// static
bool MachGui::getCDRomDriveContainingFile(string& cdRomDrive, const string& fileToCheckFor)
{
    char buffer[255];

    //  GetLogicalDriveStrings( sizeof(buffer) - 1, buffer );

    int nBuffer = 0;
    char* bufferPos = &buffer[nBuffer];

    bool found = false;

    /*  while ( bufferPos and not found )
    {
        NEIL_STREAM( "checking " << bufferPos << std::endl );
        if ( GetDriveType( bufferPos ) == DRIVE_CDROM )
        {
            NEIL_STREAM( "CD-ROM " << bufferPos << std::endl );
            string cdRomFile( bufferPos );
            cdRomFile += fileToCheckFor;
            SysPathName sysFileToCheckFor( cdRomFile );

            NEIL_STREAM( "CD-ROM checking for " << sysFileToCheckFor.c_str() << std::endl );

            if ( sysFileToCheckFor.existsAsFile() )
            {
                NEIL_STREAM( "CD-ROM found file" << std::endl );
                cdRomDrive = bufferPos;
                found = true;
            }
        }

        while ( bufferPos and not found )
        {
            ++nBuffer;
            bufferPos = &buffer[nBuffer];
            if ( strcmp( bufferPos, "" ) == 0 )
            {
                ++nBuffer;
                bufferPos = &buffer[nBuffer];

                if ( strcmp( bufferPos, "" ) == 0 )
                {
                    bufferPos = NULL;
                }
                else
                {
                    break;
                }
            }
        }
    }

    NEIL_STREAM( "CD-ROM " << found << " " << cdRomDrive << std::endl );
*/
    return found;
}

// static
const string& MachGui::wonSymbolText()
{
    static const char wonChar[2] = { (char)127, 0 };
    // static string wonSymbol( (char)127 );
    static string wonSymbol(wonChar);

    return wonSymbol;
}

/* End GUI.CPP ******************************************************/
