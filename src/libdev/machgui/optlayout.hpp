/*
 * O P T L A Y O U T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiOptionsLayout

    // Loads gui layout details from a file to specify where items will appear on screen
*/

#ifndef _OPTLAYOUT_HPP
#define _OPTLAYOUT_HPP

#include "base/base.hpp"
#include "utility/linetok.hpp"
#include "mathex/point2d.hpp"

class MachGuiOptionsLayout
// Canonical form revoked
{
public:
    struct SlidebarInfo
    {
        SlidebarInfo(MexPoint2d tl, uint rng)
            : topLeft(tl)
            , range(rng) {};
        MexPoint2d topLeft;
        uint range;
    };

    struct MenuButtonInfo
    {
        MenuButtonInfo(MexPoint2d tl, MexPoint2d br)
            : topLeft(tl)
            , bottomRight(br)
        {
        }
        MexPoint2d topLeft;
        MexPoint2d bottomRight;
    };

    struct MenuTextInfo
    {
        MenuTextInfo(uint stringId, MexPoint2d tl, MexPoint2d br, const string& fnt)
            : idsStringId(stringId)
            , topLeft(tl)
            , bottomRight(br)
            , font(fnt) {};
        uint idsStringId;
        MexPoint2d topLeft;
        MexPoint2d bottomRight;
        string font;
    };

    struct CheckBoxInfo
    {
        CheckBoxInfo(MexPoint2d tl, uint id)
            : topLeft(tl)
            , stringId(id) {};
        MexPoint2d topLeft;
        uint stringId;
    };

    MachGuiOptionsLayout(const SysPathName&, float scale = 1);
    ~MachGuiOptionsLayout();

    const MachGuiOptionsLayout::SlidebarInfo& slidebarInfo(uint index) const;
    // PRE( index < nScrollBars() )

    const MachGuiOptionsLayout::MenuButtonInfo& menuButtonInfo(uint index) const;
    const MachGuiOptionsLayout::MenuTextInfo& menuTextInfo(uint index) const;
    // PRE( index < nMenuTexts() )

    const MachGuiOptionsLayout::CheckBoxInfo& checkBoxInfo(uint index) const;
    // PRE( index < nCheckBoxes() )

    uint nSlidebars() const { return slidebars_.size(); }
    uint nMenuButtons() const { return menuButtons_.size(); }
    uint nMenuTexts() const { return menuTexts_.size(); }
    uint nCheckBoxes() const { return checkBoxes_.size(); }

    void CLASS_INVARIANT;

private:
    void parse();
    void parseSlidebar(const UtlLineTokeniser::Tokens& tokens);
    void parseMenuButton(const UtlLineTokeniser::Tokens& tokens);
    void parseMenuText(const UtlLineTokeniser::Tokens& tokens);
    void parseCheckBox(const UtlLineTokeniser::Tokens& tokens);

    friend std::ostream& operator<<(std::ostream& o, const MachGuiOptionsLayout& t);

    MachGuiOptionsLayout(const MachGuiOptionsLayout&);
    MachGuiOptionsLayout& operator=(const MachGuiOptionsLayout&);

    std::vector<SlidebarInfo> slidebars_;
    std::vector<MenuButtonInfo> menuButtons_;
    std::vector<MenuTextInfo> menuTexts_;
    std::vector<CheckBoxInfo> checkBoxes_;
    UtlLineTokeniser parser_;
    float scale_{};
};

#endif

/* End OPTLAYOUT.HPP ************************************************/
