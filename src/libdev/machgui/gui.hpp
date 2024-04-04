/*
 * G U I . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#pragma once

#include "gui/gui.hpp"
#include "stdlib/string.hpp"

class MachLogMachine;

namespace MachGui
{

// Load the correct icon for a machine. You may specify whether it's normal looking or matrix-like (first person)
GuiBitmap createIconForMachine(MachLogMachine* machine, bool firstPersonIcon);

// Draws a number onto the screen. numberArray is an array of bitmaps, each
// bitmap representing a number. The index position in the array is the same
// as the number shown in the bitmap.
void drawNumber(GuiBitmap* pNumberArray, size_t value, Gui::Coord* pAbsCoord);
// PRE( pAbsCoord );

const Gui::Colour& mVIIIBackgroundColour();
const Gui::Colour& OFFWHITE();
const Gui::Colour& VERYDARKGREY();
const Gui::Colour& ALMOSTBLACK();
const Gui::Colour& VERYDARKPURPLE();
const Gui::Colour& LIGHTPURPLE();
const Gui::Colour& PURPLE();
const Gui::Colour& PROGRESSGREEN();

const Gui::Colour& MENUDARKGREEN();
const Gui::Colour& SANDY();
const Gui::Colour& DARKSANDY();
const Gui::Colour& DARKRED();
const Gui::Colour& DARKYELLOW();
const Gui::Colour& DARKGREEN();
const Gui::Colour& DARKBLUE();
const Gui::Colour& DROPDOWNDARKGREEN();
const Gui::Colour& DROPDOWNLIGHTGREEN();

void releaseMenuBmpMemory();
// Cached menu bitmaps
GuiBitmap& longGlowBmp();
GuiBitmap& longYellowGlowBmp();
GuiBitmap& buttonGlowBmp();
GuiBitmap& buttonDisableBmp();
GuiBitmap& menuScrollUpBmp();
GuiBitmap& menuScrollDownBmp();
GuiBitmap& okMsgBoxBmp();
GuiBitmap& okCancelMsgBoxBmp();
GuiBitmap& tickBmp();
GuiBitmap& darkTickBmp();
GuiBitmap& largeTickBmp();
GuiBitmap& largeDarkTickBmp();
GuiBitmap& dropDownBmp();
GuiBitmap& hostBmp();
GuiBitmap& slideBtnBmp();
GuiBitmap& controlPanelBmp();
GuiBitmap& inTransitBmp();

const string& closedText();
const string& openText();
const string& computerText();
const string& unavailableText();
const string& wonSymbolText();

MATHEX_SCALAR uiScaleFactor();
MATHEX_SCALAR menuScaleFactor();
void setUiScaleFactor(MATHEX_SCALAR scale);

std::string getScaledImagePath(std::string path);

/**
 * @brief Returns a bitmap scaled by uiScaleFactor() or a HiDPI alternatives
 * @param path to the image file (with .bmp or without any extension)
 * @return scaled image or HiDPI bitmap
 */
GuiBitmap getScaledImage(std::string path);

float getPhysMarkerLineWidth();

// Various gui layout values
int controlPanelInXPos();
int controlPanelOutXPos();
int controlPanelSlideInSpeed();
int controlPanelSlideOutSpeed();
int promptTextXOffset();
int promptTextYOffset();
int mapAreaHeight();

int barBorderThickness();
int barShadowThickness();
int barDividerThickness();
int barValueLineOffset();
int barValueLineThickness();

int iconIndexYOffset();

int corralInfoTextSize();
int navigationButtonNumberSize();

double doubleClickInterval();

// Context enumerations
enum ControlPanelContext
{
    MAIN_MENU,
    SQUADRON_MENU,
    MACHINE_NAVIGATION_MENU,
    CONSTRUCTION_NAVIGATION_MENU,
    CONSTRUCT_COMMAND,
    BUILD_COMMAND,
    HARDWARE_RESEARCH,
    SOFTWARE_RESEARCH,
    FORM_SQUADRON_COMMAND,
    SINGLE_FACTORY
};

// Cursor enumeration
enum Cursor2dType
{
    MENU_CURSOR,
    INVALID_CURSOR,
    MOVETO_CURSOR,
    ATTACK_CURSOR,
    CONSTRUCT_CURSOR,
    LOCATETO_CURSOR,
    PATROL_CURSOR,
    DEPLOY_CURSOR,
    PICKUP_CURSOR,
    TRANSPORT_CURSOR,
    ENTER_BUILDING_CURSOR,
    ENTER_APC_CURSOR,
    FOLLOW_CURSOR,
    SELECT_CURSOR,
    DROPLANDMINE_CURSOR,
    HEAL_CURSOR,
    DECONSTRUCT_CURSOR,
    RECYCLE_CURSOR,
    STANDGROUND_CUSROR,
    REPAIR_CURSOR,
    CAPTURE_CURSOR,
    JOINCONSTRUCT_CURSOR,
    ASSEMBLEPOINT_CURSOR,
    ION_ATTACK_CURSOR,
    NUKE_ATTACK_CURSOR,
    SCROLL_N_CURSOR,
    SCROLL_S_CURSOR,
    SCROLL_E_CURSOR,
    SCROLL_W_CURSOR,
    SCROLL_NE_CURSOR,
    SCROLL_NW_CURSOR,
    SCROLL_SE_CURSOR,
    SCROLL_SW_CURSOR,
    SCAVENGE_CURSOR,
    CHOOSE_CONST_CURSOR,
    TREACHERY_CURSOR
};

int convertNetworkError(int);

void releaseInGameBmpMemory();

// Funtions that check for existance of CD
bool getCDRomDriveContainingFile(string& cdRomDrive, const string& fileToCheckFor);

bool getCDRomDriveContainingMachinesCD(string& cdRomDrive, int CDNumber);
// PRE( CDNumber > and 0 CDNumber < 3 );

bool machinesCDIsAvailable(int CDNumber);
// PRE( CDNumber > and 0 CDNumber < 3 );

}; // namespace MachGui
