/*
 * S L I D E B A R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiSlideBar

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_SLIDEBAR_HPP
#define _MACHGUI_SLIDEBAR_HPP

#include "base/base.hpp"
#include "gui/displaya.hpp"
#include "machgui/focusctl.hpp"

#include <functional>

class MachGuiStartupScreens;

class MachGuiSlideBar
    : public GuiDisplayable
    , public MachGuiFocusCapableControl
// Canonical form revoked
{
public:
    using FloatValueChangedCallback = std::function<void(float newValue)>;

    MachGuiSlideBar(
        MachGuiStartupScreens*,
        GuiDisplayable* pParent,
        Gui::Coord topLeft,
        size_t width,
        float minVal,
        float maxVal);
    // POST( value() >= minValue_ );
    // POST( value() <= maxValue_ );

    MachGuiSlideBar(MachGuiStartupScreens*, GuiDisplayable* pParent, Gui::Coord topLeft, size_t width);
    // POST( minValue_ == 0.0 );
    // POST( maxValue_ == 1.0 );
    // POST( value() == minValue_ );

    ~MachGuiSlideBar() override;

    // Set the range of values the slide bar can produce.
    void minMax(float minV, float maxV);
    // PRE( minV < maxV );

    // Get value slide bar is pointing at ( between min and max values ).
    float value() const;

    // Change the value, this will cause the slide bar to be redrawn.
    void setValue(float);
    // PRE( newVal >= minValue_ and newVal <= maxValue_ );

    // Called when the slide button is moved into a new position.
    virtual void valueChanged(float value);

    void setValueChangedHandler(FloatValueChangedCallback callback);

    static size_t reqHeight();

    bool doHandleNavigationKey(NavKey, MachGuiFocusCapableControl**) override;

    void hasFocus(bool newValue) override;

    void CLASS_INVARIANT;

protected:
    void doDisplay() override;

    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doHandleContainsMouseEvent(const GuiMouseEvent& rel) override;

    void barMoved(Gui::XCoord newBarPos);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiSlideBar& t);

    MachGuiSlideBar(const MachGuiSlideBar&);
    MachGuiSlideBar& operator=(const MachGuiSlideBar&);

    FloatValueChangedCallback valueChangedCallback_;

    float barPos_;
    float minValue_;
    float maxValue_;
    bool highlighted_;
};

#endif

/* End SLIDEBAR.HPP *************************************************/
