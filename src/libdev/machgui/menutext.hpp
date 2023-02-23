/*
 * M E N U T E X T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiMenuText

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_MENUTEXT_HPP
#define _MACHGUI_MENUTEXT_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "ctl/vector.hpp"
#include "gui/displaya.hpp"
#include "system/pathname.hpp"

using strings = ctl_vector<string>;
class GuiBmpFont;

class MachGuiMenuText : public GuiDisplayable
// Canonical form revoked
{
public:
    enum Justification
    {
        LEFT_JUSTIFY,
        RIGHT_JUSTIFY,
        CENTRE_JUSTIFY
    };

    MachGuiMenuText(
        GuiDisplayable* pParent,
        const Gui::Box& box,
        unsigned int stringId,
        const SysPathName& fontPath,
        Justification = CENTRE_JUSTIFY);
    MachGuiMenuText(
        GuiDisplayable* pParent,
        const Gui::Box& box,
        const string&,
        const SysPathName& fontPath,
        Justification = CENTRE_JUSTIFY);
    ~MachGuiMenuText() override;

    void CLASS_INVARIANT;

    static void chopUpText(const string& text, size_t maxWidth, const GuiBmpFont& font, strings* pStrings);

protected:
    void doDisplay() override;

private:
    friend ostream& operator<<(ostream& o, const MachGuiMenuText& t);

    MachGuiMenuText(const MachGuiMenuText&);
    MachGuiMenuText& operator=(const MachGuiMenuText&);

    SysPathName fontPath_;
    strings strings_;
    Justification justification_;
};

#endif

/* End MENUTEXT.HPP *************************************************/
