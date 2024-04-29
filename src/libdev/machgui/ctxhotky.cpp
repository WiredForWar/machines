/*
 * C T X H O T K Y . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxhotky.hpp"
#include "ctl/vector.hpp"
#include "gui/font.hpp"
#include "system/pathname.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/ingame.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/ScrollableText.hpp"
#include "machgui/internal/strings.hpp"
#include "utility/linetok.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "ani/AniSmacker.hpp"
#include "ani/AniSmackerRegular.hpp"
#include "device/cd.hpp"
#include <fstream>
#include <algorithm>

MachGuiCtxHotKeys::MachGuiCtxHotKeys(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("so", pStartupScreens)
{
    const uint HOTKEY_MIN_X = MachGui::menuScaleFactor() * 128;
    const uint HOTKEY_MIN_Y = MachGui::menuScaleFactor() * 15;
    const uint HOTKEY_MAX_Y = MachGui::menuScaleFactor() * 350;
    const uint HOTKEY_ACTION_WIDTH = MachGui::menuScaleFactor() * 111;
    const uint HOTKEY_KEY_WIDTH = MachGui::menuScaleFactor() * 131;
    const uint HOTKEY_SEPARATION = MachGui::menuScaleFactor() * 2;

    const uint HOTKEY_KEY_X = HOTKEY_MIN_X + HOTKEY_ACTION_WIDTH;
    const uint HOTKEY_2NDCOLUMN_X = HOTKEY_MIN_X + HOTKEY_ACTION_WIDTH + HOTKEY_KEY_WIDTH + HOTKEY_SEPARATION;

    uint smallFontHeight
        = GuiBmpFont::getFont(MachGui::Menu::smallFontLight()).charHeight() + MachGui::menuScaleFactor() * 2;
    uint largeFontHeight = GuiBmpFont::getFont(SysPathName(MachGui::Menu::largeFontLight())).charHeight()
        + MachGui::menuScaleFactor() * 2;

    bool enableAnimation = MachGui::menuScaleFactor() == 1;

    // Display First Person Control heading
    //
    GuiBmpFont font = GuiBmpFont::getFont(SysPathName(MachGui::Menu::largeFontLight()));
    GuiResourceString optionsHeading(IDS_MENU_FIRSTPERSONCONTROL);
    MachGuiMenuText* firstPersonHeading = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(HOTKEY_MIN_X, HOTKEY_MIN_Y),
            Gui::Size(font.textWidth(optionsHeading.asString()), largeFontHeight)),
        IDS_MENU_FIRSTPERSONCONTROL,
        MachGui::Menu::largeFontLight(),
        Gui::AlignLeft|Gui::AlignTop);

    // Create First Person Actions text below First Person Control heading
    //
    uint noLines;
    string hotKey1stPersonActions;
    readHotkeyData("gui/menu/hk1pAction.dat", hotKey1stPersonActions, noLines);

    MachGuiMenuText* firstPersonContent = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            firstPersonHeading->relativeBoundary().bottomLeft(),
            Gui::Size(HOTKEY_ACTION_WIDTH, noLines * smallFontHeight)),
        hotKey1stPersonActions,
        MachGui::Menu::smallFontLight(),
        Gui::AlignLeft|Gui::AlignTop);

    uint headingMaxY = firstPersonHeading->relativeBoundary().bottom();
    uint fstPersonWindowMaxY = firstPersonContent->relativeBoundary().bottom();

    // Create First Person Keys text next to First Person Action text
    //
    string hotKey1stPersonKeys;
    readHotkeyData("gui/menu/hk1pKeys.dat", hotKey1stPersonKeys, noLines);
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(HOTKEY_KEY_X, headingMaxY, HOTKEY_KEY_X + HOTKEY_KEY_WIDTH, fstPersonWindowMaxY),
        hotKey1stPersonKeys,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft|Gui::AlignTop);

    // Display General Controls heading underneath First Person Controls hot keys
    //
    font = GuiBmpFont::getFont(SysPathName(MachGui::Menu::largeFontLight()));
    GuiResourceString optionsGeneralHeading(IDS_MENU_GENERALCONTROL);
    uint genHeadingMaxY = fstPersonWindowMaxY + largeFontHeight;

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            HOTKEY_MIN_X,
            fstPersonWindowMaxY,
            HOTKEY_MIN_X + font.textWidth(optionsGeneralHeading.asString()),
            genHeadingMaxY),
        IDS_MENU_GENERALCONTROL,
        MachGui::Menu::largeFontLight(),
        Gui::AlignLeft|Gui::AlignTop);

    // Calculate the number of lines of General hotkeys that can be displayed under
    // the First Person hotkeys
    font = GuiBmpFont::getFont(MachGui::Menu::smallFontLight());

    uint noDisplayableLines = (HOTKEY_MAX_Y - genHeadingMaxY) / smallFontHeight;

    string hotKeyGeneralActions;

    readHotkeyData("gui/menu/hkGenActions.dat", hotKeyGeneralActions, noLines);

    strings choppedText;

    MachGuiMenuText::chopUpText(hotKeyGeneralActions, MachGui::menuScaleFactor() * 200, font, &choppedText);

    uint noRemainingLines = noLines - noDisplayableLines;

    WAYNE_STREAM(" noDisplayableLines " << noDisplayableLines << std::endl);
    WAYNE_STREAM(" noRemainingLines - " << noRemainingLines << std::endl);

    string headString, remainderString;

    // Create new strings to be displayed
    for (uint i = 0; i < noDisplayableLines; i++)
    {
        headString += choppedText[i];
        headString += "\n";
    }
    uint stringSize = choppedText.size();

    for (uint i = noDisplayableLines; i < stringSize; i++)
    {
        remainderString += choppedText[i];
        remainderString += "\n";
    }
    WAYNE_STREAM("headString - " << headString << std::endl);
    WAYNE_STREAM("remainderString - " << remainderString << std::endl);

    uint generalWindowMaxY = genHeadingMaxY + (noDisplayableLines * smallFontHeight);

    WAYNE_STREAM("generalHeadingMaxY - " << genHeadingMaxY << std::endl);
    WAYNE_STREAM("generalWindowMaxY - " << generalWindowMaxY << std::endl);

    WAYNE_STREAM(
        "Head box dimensions: " << HOTKEY_MIN_X << "," << genHeadingMaxY << "," << HOTKEY_MIN_X << ","
                                << generalWindowMaxY << std::endl);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(HOTKEY_MIN_X, genHeadingMaxY, HOTKEY_KEY_X, generalWindowMaxY),
        headString,
        MachGui::Menu::smallFontLight(),
        Gui::AlignLeft|Gui::AlignTop);

    const int secondColumnY = enableAnimation ? HOTKEY_MIN_Y : firstPersonContent->relativeBoundary().top();
    uint generalRemainderHeight = noRemainingLines * smallFontHeight;

    WAYNE_STREAM("generalRemainderMaxY - " << generalRemainderHeight << std::endl);

    WAYNE_STREAM(
        " Remainder box dimensions: " << HOTKEY_2NDCOLUMN_X << "," << HOTKEY_MIN_Y << "," << HOTKEY_2NDCOLUMN_X << ","
                                      << generalRemainderHeight << std::endl);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(Gui::Coord(HOTKEY_2NDCOLUMN_X, secondColumnY), Gui::Size(HOTKEY_ACTION_WIDTH, generalRemainderHeight)),
        remainderString,
        MachGui::Menu::smallFontLight(),
        Gui::AlignLeft|Gui::AlignTop);

    string hotKeyGeneralKeys;
    readHotkeyData("gui/menu/hkGenKeys.dat", hotKeyGeneralKeys, noLines);

    strings choppedupText;
    MachGuiMenuText::chopUpText(hotKeyGeneralKeys, MachGui::menuScaleFactor() * 200, font, &choppedupText);

    headString = remainderString = "";

    // Create new strings to be displayed
    for (uint i = 0; i < noDisplayableLines; i++)
    {
        headString += choppedupText[i];
        headString += "\n";
    }
    for (uint i = noDisplayableLines; i < stringSize; i++)
    {
        remainderString += choppedupText[i];
        remainderString += "\n";
    }
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(HOTKEY_KEY_X, genHeadingMaxY, HOTKEY_KEY_X + HOTKEY_KEY_WIDTH, generalWindowMaxY),
        headString,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft|Gui::AlignTop);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(HOTKEY_2NDCOLUMN_X + HOTKEY_ACTION_WIDTH, secondColumnY),
            Gui::Size(HOTKEY_KEY_WIDTH, generalRemainderHeight)),
        remainderString,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft|Gui::AlignTop);

    if (enableAnimation)
    {
        // Add flick to bottom left of window
        SysPathName hotkeySmackerFile("flics/gui/hotkeys.smk");

        // Get flic off hard-disk or CD-Rom
        if (!hotkeySmackerFile.existsAsFile())
        {
            // Make sure the cd is stopped before accessing files on it.
            if (DevCD::instance().isPlayingAudioCd())
            {
                DevCD::instance().stopPlaying();
            }

            string cdRomDrive;

            if (MachGui::getCDRomDriveContainingFile(cdRomDrive, "flics/gui/hotkeys.smk"))
            {
                hotkeySmackerFile = SysPathName(cdRomDrive + "flics/gui/hotkeys.smk");

                // Can't play music and smacker anim off CD at same time
                if (hotkeySmackerFile.existsAsFile())
                {
                    pStartupScreens_->desiredCdTrack(MachGuiStartupScreens::DONT_PLAY_CD);
                }
            }
        }

        // Play animation only if it exists
        if (hotkeySmackerFile.existsAsFile())
        {
            const auto& topLeft = getBackdropTopLeft();
            AniSmacker* pSmackerAnimation = new AniSmackerRegular(
                hotkeySmackerFile,
                MachGui::menuScaleFactor() * 430 + topLeft.second,
                MachGui::menuScaleFactor() * 199 + topLeft.first);
            pSmackerAnimation->setScaleFactor(MachGui::menuScaleFactor());
            pStartupScreens_->addSmackerAnimation(pSmackerAnimation);
        }
    }

    MachGuiMenuButton* pContinueBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(351, 420, 553, 464) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CONTINUE,
        MachGui::ButtonEvent::EXIT);
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(42, 353, 224, 464) * MachGui::menuScaleFactor(),
        IDS_MENU_HOTKEYS,
        MachGui::Menu::largeFontLight());

    pContinueBtn->escapeControl(true);

    TEST_INVARIANT;
}

MachGuiCtxHotKeys::~MachGuiCtxHotKeys()
{
    TEST_INVARIANT;
}

void MachGuiCtxHotKeys::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxHotKeys& t)
{

    o << "MachGuiCtxHotKeys " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxHotKeys " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxHotKeys::update()
{
    animations_.update();
}

void MachGuiCtxHotKeys::readHotkeyData(const string& hotKeyDataFileName, string& hotkeyString, uint& linesInString)
{
    SysPathName hotKeyFilePath = SysPathName(hotKeyDataFileName);
    string path = string(hotKeyDataFileName.c_str());

    if (hotKeyFilePath.containsCapitals() && !hotKeyFilePath.existsAsFile())
    {
        std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    ASSERT(hotKeyFilePath.insensitiveExistsAsFile(), hotKeyFilePath.c_str());
    std::ifstream hotKeyFile(path.c_str());
    char nextChar;
    uint noLines = 0;

    while (hotKeyFile.get(nextChar))
    {
        if (nextChar == '\n')
            ++noLines;
        hotkeyString += nextChar;
    }
    linesInString = noLines;
}
/* End CTXHOTKY.CPP *************************************************/
