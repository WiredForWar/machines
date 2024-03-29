
#ifndef _GUI_BUTTON_HPP
#define _GUI_BUTTON_HPP

#include "gui/gui.hpp"
#include "gui/border.hpp"
#include "gui/displaya.hpp"
#include <functional>

/* //////////////////////////////////////////////////////////////// */

class GuiButton : public GuiDisplayable
{
public:
    using Callback = std::function<void(GuiButton*)>;

    GuiButton(GuiDisplayable* pParent, const Gui::Box& rel);

    // popupButton should be set to false if you want the button to stay
    // depressed when clicked.
    void popupButton(bool);

    // Force the button into a depressed or released state. Note that doBeDepressed
    // and doBeReleased are called with coord(0,0)
    void setDepressed(bool);

    bool isDepressed() const;
    bool isDisabled() const;

    // inherited from GuiDisplayable...
    void doDisplay() override;
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;

    void setMouseClickHandler(Callback callback);

protected:
    ~GuiButton() override;

    virtual void doBeDepressed(const GuiMouseEvent& rel) = 0;
    virtual void doBeReleased(const GuiMouseEvent& rel) = 0;

    virtual void doDisplayDepressedEnabled() = 0;
    virtual void doDisplayReleasedEnabled() = 0;

    // TBD: the next two should also be pure virtual, but they're
    // implemented in terms of the other two for now
    virtual void doDisplayDepressedDisabled();
    virtual void doDisplayReleasedDisabled();

    void enable();
    // PRE( isDisabled() );
    // PRE( not isDisabled() );

    void disable();
    // PRE( not isDisabled() );
    // POST( isDisabled() );

private:
    GuiButton(const GuiButton&);
    GuiButton& operator=(const GuiButton&);
    bool operator==(const GuiButton&) const;

    std::function<void(GuiButton*)> clickedCallback_;

    bool isDepressed_;
    bool isDisabled_;
    bool isPopupButton_;
};

//////////////////////////////////////////////////////////////////////

class GuiButtonWithBorder : public GuiButton
{
public:
    GuiButtonWithBorder(
        GuiDisplayable* pParent,
        const Gui::Box& rel,
        const GuiBorderMetrics&,
        const Gui::Coord& depressedInteriorOffset);
    // POST( borderDimensions().exteriorWidth() == Gui::width( rel ) );
    // POST( borderDimensions().exteriorHeight() == Gui::height( rel ) );

    ~GuiButtonWithBorder() override;

protected:
    void doDisplayDepressedEnabled() override;
    void doDisplayReleasedEnabled() override;

    virtual void doDisplayBorderDepressedEnabled() = 0;
    virtual void doDisplayBorderReleasedEnabled() = 0;

    // TBD: the next two should also be pure virtual, but they're
    // implemented in terms of the other two for now
    virtual void doDisplayBorderDepressedDisabled(); // = 0;
    virtual void doDisplayBorderReleasedDisabled(); // = 0;

    virtual void doDisplayInteriorEnabled(const Gui::Coord& absCoord) = 0;
    // virtual void doDisplayInteriorDisabled( const Gui::Coord& absCoord ) = 0;

    static GuiBorderDimensions interiorDimensions(const Gui::Box& rel, const GuiBorderMetrics&);

    const GuiBorderMetrics& borderMetrics() const;
    const GuiBorderDimensions& borderDimensions() const;

    const Gui::Coord& depressedInteriorOffset() const;

private:
    GuiButtonWithBorder(const GuiButtonWithBorder&);
    GuiButtonWithBorder& operator=(const GuiButtonWithBorder&);
    bool operator==(const GuiButtonWithBorder&) const;

    GuiBorderMetrics borderMetrics_;
    GuiBorderDimensions borderDimensions_;
    Gui::Coord depressedInteriorOffset_;
};

//////////////////////////////////////////////////////////////////////

class GuiButtonWithFilledBorder : public GuiButtonWithBorder
{
public:
    GuiButtonWithFilledBorder(
        GuiDisplayable* pParent,
        const Gui::Box& rel,
        const GuiBorderMetrics&,
        const GuiFilledBorderColours&,
        const Gui::Coord& depressedInteriorOffset);

    ~GuiButtonWithFilledBorder() override;

protected:
    void doDisplayBorderDepressedEnabled() override;
    void doDisplayBorderReleasedEnabled() override;

    GuiFilledBorder& border();
    const GuiFilledBorder& border() const;

private:
    GuiButtonWithFilledBorder(const GuiButtonWithFilledBorder&);
    GuiButtonWithFilledBorder& operator=(const GuiButtonWithFilledBorder&);
    bool operator==(const GuiButtonWithFilledBorder&) const;

    GuiFilledBorder border_;
};

//////////////////////////////////////////////////////////////////////

class GuiBitmapButtonWithFilledBorder : public GuiButtonWithFilledBorder
{
public:
    GuiBitmapButtonWithFilledBorder(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const GuiBorderMetrics&,
        const GuiFilledBorderColours&,
        const GuiBitmap&,
        const Gui::Coord& depressedInteriorOffset);

    ~GuiBitmapButtonWithFilledBorder() override;

    static Gui::Box exteriorRelativeBoundary(const GuiBitmap& bitmap, const GuiBorderMetrics& m, const Gui::Coord& rel);

    // Change buttons bitmap
    void bitmap(const GuiBitmap&);

protected:
    void doDisplayInteriorEnabled(const Gui::Coord& absCoord) override;

    virtual const GuiBitmap& getBitmap() const;

private:
    GuiBitmapButtonWithFilledBorder(const GuiBitmapButtonWithFilledBorder&);
    GuiBitmapButtonWithFilledBorder& operator=(const GuiBitmapButtonWithFilledBorder&);
    bool operator==(const GuiButtonWithFilledBorder&) const;

    GuiBitmap bitmap_;
};

//////////////////////////////////////////////////////////////////////

#endif // #ifndef _GUI_BUTTON_HPP

/* End BUTTON.HPP ***************************************************/
