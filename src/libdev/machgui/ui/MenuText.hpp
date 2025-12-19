/*
 * M E N U T E X T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#pragma once

#include "gui/displaya.hpp"

#include "system/pathname.hpp"
#include "gui/ResolvedUiString.hpp"
#include "render/TextOptions.hpp"

using strings = std::vector<std::string>;
class GuiBmpFont;

class MachGuiMenuText : public GuiDisplayable
// Canonical form revoked
{
public:
    MachGuiMenuText(
        GuiDisplayable* pParent,
        const Gui::Box& box,
        const ResolvedUiString& str,
        const SysPathName& bitmapFontPath,
        Gui::Alignment alignment = Gui::AlignCenter);

    MachGuiMenuText(
        GuiDisplayable* pParent,
        const Gui::Box& box,
        const ResolvedUiString& str,
        const Render::Font& font,
        const Render::TextOptions& options,
        Gui::Alignment alignment = Gui::AlignCenter);
    ~MachGuiMenuText() override;

    void CLASS_INVARIANT;

    static void chopUpText(const std::string& text, size_t maxWidth, const GuiBmpFont& font, strings* pStrings);
    static void chopUpText(
        const std::string& text,
        size_t maxWidth,
        const Render::Font& font,
        const Render::TextOptions& options,
        strings* pStrings);

protected:
    void doDisplay() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiMenuText& t);

    MachGuiMenuText(const MachGuiMenuText&);
    MachGuiMenuText& operator=(const MachGuiMenuText&);

    SysPathName bitmapFontPath_;
    int fontHeight_{};
    const Render::Font* font_{};
    const Render::TextOptions textOptions_;
    strings strings_;
    Gui::Alignment alignment_{};
};
