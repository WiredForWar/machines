/*
 * C T X H O T K Y . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/MenuContext/CtxHotKeys.hpp"

#include "gui/Font.hpp"
#include "system/PathName.hpp"
#include "machgui/InputRegistry.hpp"
#include "machgui/gui.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "render/Font.hpp"
#include "render/Device.hpp"
#include "render/Display.hpp"
#include "ani/AniSmacker.hpp"
#include "ani/AniSmackerRegular.hpp"


#include "utility/String.hpp"

#include <algorithm>
#include <fstream>

MachGuiCtxHotKeys::MachGuiCtxHotKeys(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("so", pStartupScreens)
{
    const uint HOTKEY_MIN_X = MachGui::menuScaleFactor() * 128;
    const uint HOTKEY_MIN_Y = MachGui::menuScaleFactor() * 15;
    const uint HOTKEY_MAX_Y = MachGui::menuScaleFactor() * 338;
    const uint HOTKEY_ACTION_WIDTH = MachGui::menuScaleFactor() * 111;
    const uint HOTKEY_KEY_WIDTH = MachGui::menuScaleFactor() * 131;
    const uint HOTKEY_SEPARATION = MachGui::menuScaleFactor() * 2;
    const uint HOTKEY_2NDCOLUMN_X = HOTKEY_MIN_X + HOTKEY_ACTION_WIDTH + HOTKEY_KEY_WIDTH + HOTKEY_SEPARATION;

    const Ren::Font& headingFont = MachGui::Menu::font();
    const GuiBmpFont textFont = Gui::getFont(MachGui::Menu::smallFontLight());
    const uint headingFontHeight = headingFont.height() + MachGui::menuScaleFactor() * 2;
    const uint textFontHeight = textFont.height() + MachGui::menuScaleFactor() * 1;
    const uint verticalSpacing = textFontHeight;

    bool enableAnimation = MachGui::menuScaleFactor() == 1;

    const MachGui::InputRegistry* inputRegistry = MachGui::inputRegistryImpl();
    uint x = HOTKEY_MIN_X;
    uint y = HOTKEY_MIN_Y;

    const uint maxLinesInColumn = (HOTKEY_MAX_Y - y - headingFontHeight) / textFontHeight;
    const uint maxLines = maxLinesInColumn * 2;

    const auto useNextColumn = [&]()
    {
        x = HOTKEY_2NDCOLUMN_X;
        y = HOTKEY_MIN_Y;
    };


    constexpr MachGui::KeysDisplayFormat displayFormat = MachGui::KeysDisplayFormat::Compact;
    const GuiBmpFont actionFont = Gui::getFont(MachGui::Menu::smallFontLight());
    const std::vector<std::string> categoryNames = inputRegistry->getCategories();
    for (const std::string& categoryName : inputRegistry->getCategories())
    {
        const MachGui::InputCategoryDetails& category = inputRegistry->getCategoryDetails(categoryName);
        const std::string& displayName = category.displayName_;

        std::vector<std::string> actionNames;
        std::vector<std::string> actionKeys;

        actionNames.reserve(category.bindIds_.size());
        actionKeys.reserve(actionNames.size());

        for (const MachGui::BindId& bindId : category.bindIds_)
        {
            const MachGui::BindDisplayData& data = inputRegistry->getBindDisplayData(bindId, displayFormat);
            if (data.displayName_.empty())
                continue;

            std::string_view displayNameView = data.displayName_.asStringView();
            if (actionFont.horizontalAdvance(displayNameView) > HOTKEY_ACTION_WIDTH)
            {
                // The descriptive name does not fit
                if (!data.compactDisplayName_.empty())
                    displayNameView = data.compactDisplayName_.asStringView();
            }

            // Check width, use compactDisplayName_ as fallback
            actionNames.push_back(std::string(displayNameView));
            actionKeys.push_back(data.displayBind_);

            if (actionNames.size() == maxLines)
            {
                // Ignore lines which do not fit
                break;
            }
        }

        if (actionNames.empty())
        {
            // Empty input category
            continue;
        }

        if (categoryName == categoryNames.back() && (x < HOTKEY_2NDCOLUMN_X))
        {
            const uint textSpaceInThisColumn = HOTKEY_MAX_Y - y - headingFontHeight;
            if (textSpaceInThisColumn / textFontHeight < actionNames.size())
            {
                if (actionNames.size() <= maxLinesInColumn)
                {
                    // We're gonna fit the next column
                    useNextColumn();
                }
            }
        }

        new MachGuiMenuText(
            pStartupScreens,
            Gui::Box(
                Gui::Coord(x, y),
                Gui::Size(HOTKEY_ACTION_WIDTH + HOTKEY_KEY_WIDTH, headingFontHeight)),
            displayName,
            MachGui::Menu::font(),
            MachGui::Menu::menuLightTextOptions(),
            Gui::AlignLeft|Gui::AlignTop);

        y += headingFontHeight;

        uint added = 0;
        while (added < actionNames.size())
        {
            uint thisColumnMaxItems = (HOTKEY_MAX_Y - y) / textFontHeight;
            uint columnLines = std::min<uint>(thisColumnMaxItems, actionNames.size() - added);

            uint verticalSize = columnLines * textFontHeight;

            std::string columnNames = Utils::join(std::next(actionNames.begin(), added), columnLines, "\n");
            std::string columnKeys = Utils::join(std::next(actionKeys.begin(), added), columnLines, "\n");

            new MachGuiMenuText(
                pStartupScreens,
                Gui::Box(
                    Gui::Coord(x, y),
                    Gui::Size(HOTKEY_ACTION_WIDTH, verticalSize)),
                columnNames,
                MachGui::Menu::smallFontLight(),
                Gui::AlignLeft|Gui::AlignTop);

            new MachGuiMenuText(
                pStartupScreens,
                Gui::Box(
                    Gui::Coord(x + HOTKEY_ACTION_WIDTH, y),
                    Gui::Size(HOTKEY_KEY_WIDTH, verticalSize)),
                columnKeys,
                MachGui::Menu::smallFontWhite(),
                Gui::AlignLeft|Gui::AlignTop);

            added += columnLines;

            y += verticalSize;
            if (added == actionNames.size())
                y += verticalSpacing;

            const bool needMoreLines = added < actionNames.size();
            if (needMoreLines || y + headingFontHeight > HOTKEY_MAX_Y)
            {
                if (x < HOTKEY_2NDCOLUMN_X)
                {
                    useNextColumn();
                    if (needMoreLines)
                    {
                        y += headingFontHeight;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }

    if (enableAnimation)
    {
        // Add flick to bottom left of window
        SysPathName hotkeySmackerFile("flics/gui/hotkeys.smk");

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
        MachGui::Menu::font(),
        MachGui::Menu::menuLightTextOptions());

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

void MachGuiCtxHotKeys::readHotkeyData(const std::string& hotKeyDataFileName, std::string& hotkeyString, uint& linesInString)
{
    SysPathName hotKeyFilePath = SysPathName(hotKeyDataFileName);
    std::string path = std::string(hotKeyDataFileName.c_str());

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
