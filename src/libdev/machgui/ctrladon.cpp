/*
 * C T R L A D O N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctrladon.hpp"
#include "machgui/gui.hpp"
#include "machgui/ingame.hpp"
#include "machgui/cameras.hpp"
#include "gui/painter.hpp"
#include "gui/gui.hpp"
#include "gui/icon.hpp"
#include "gui/restring.hpp"
#include "system/pathname.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/internal/mgsndman.hpp"

///////////////////////////////////////////////////////////////////////////////

class MachGuiCameraToggleBtn : public GuiBitmapButtonWithFilledBorder
// cannonical from revoked
{
public:
    MachGuiCameraToggleBtn(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen)
        : GuiBitmapButtonWithFilledBorder(
            pParent,
            rel,
            GuiBorderMetrics(1 * MachGui::uiScaleFactor(), 1 * MachGui::uiScaleFactor(), 1 * MachGui::uiScaleFactor()),
            GuiFilledBorderColours(
                Gui::Colour(144.0 / 255.0, 148.0 / 255.0, 160.0 / 255.0),
                Gui::Colour(232.0 / 255.0, 232.0 / 255.0, 232.0 / 255.0),
                Gui::Colour(62.0 / 255.0, 62.0 / 255.0, 62.0 / 255.0),
                Gui::RED()),
            MachGui::getScaledImage("gui/misc/zenith.bmp"),
            Gui::Coord(1, 1))
        , pInGameScreen_(pInGameScreen)
    {
    }

protected:
    // inherited from GuiIcon...
    void doBeReleased(const GuiMouseEvent&) override
    {
        if (pInGameScreen_->cameras()->isZenithCameraActive())
            pInGameScreen_->cameras()->useGroundCamera();
        else
            pInGameScreen_->cameras()->useZenithCamera();
    }

    void doBeDepressed(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
    }

    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override
    {
        GuiBitmapButtonWithFilledBorder::doHandleMouseEnterEvent(rel);

        usingZenithCamera_ = pInGameScreen_->cameras()->isZenithCameraActive();
        // Load the resource string
        GuiResourceString prompt(usingZenithCamera_ ? IDS_GROUND_CAMERA : IDS_ZENITH_CAMERA);

        // Set the cursor prompt
        pInGameScreen_->cursorPromptText(prompt.asString());
    }

    const GuiBitmap& getBitmap() const override
    {
        static GuiBitmap zenithBmp = MachGui::getScaledImage("gui/misc/zenith.bmp");
        static GuiBitmap groundBmp = MachGui::getScaledImage("gui/misc/ground.bmp");

        if (pInGameScreen_->cameras()->isZenithCameraActive())
            return groundBmp;

        return zenithBmp;
    }

    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override
    {
        GuiBitmapButtonWithFilledBorder::doHandleMouseExitEvent(rel);

        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();
    }

    void doHandleContainsMouseEvent(const GuiMouseEvent&) override
    {
        // Change prompt text if keyboard has been used to switch cameras.
        if (usingZenithCamera_ != pInGameScreen_->cameras()->isZenithCameraActive())
        {
            usingZenithCamera_ = not usingZenithCamera_;

            // Load the resource string
            GuiResourceString prompt(usingZenithCamera_ ? IDS_GROUND_CAMERA : IDS_ZENITH_CAMERA);

            // Set the cursor prompt
            pInGameScreen_->cursorPromptText(prompt.asString());
        }
    }

private:
    MachGuiCameraToggleBtn(const MachGuiCameraToggleBtn&);
    MachGuiCameraToggleBtn& operator=(const MachGuiCameraToggleBtn&);
    bool operator==(const MachGuiCameraToggleBtn&) const;

    // Data members...
    MachInGameScreen* pInGameScreen_;
    bool usingZenithCamera_;
};

///////////////////////////////////////////////////////////////////////////////

class MachGuiReturnToMenuBtn : public GuiButton
// cannonical from revoked
{
public:
    MachGuiReturnToMenuBtn(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen)
        : GuiButton(pParent, Gui::Box(rel, Gui::Size(20, 20) * MachGui::uiScaleFactor()))
        , pInGameScreen_(pInGameScreen)
        , return1Bmp_(MachGui::getScaledImage("gui/misc/return1.bmp"))
        , return2Bmp_(MachGui::getScaledImage("gui/misc/return2.bmp"))
    {
    }

protected:
    // inherited from GuiIcon...
    void doBeReleased(const GuiMouseEvent&) override { pInGameScreen_->switchToMenus(true); }

    void doBeDepressed(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
    }

    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override
    {
        GuiButton::doHandleMouseEnterEvent(rel);

        // Load the resource string
        GuiResourceString prompt(IDS_RETURNTOMENUS);

        // Set the cursor prompt
        pInGameScreen_->cursorPromptText(prompt.asString());
    }

    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override
    {
        GuiButton::doHandleMouseExitEvent(rel);

        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();
    }

    void doDisplayDepressedEnabled() override
    {
        GuiPainter::instance().blit(return2Bmp_, absoluteBoundary().minCorner());
    }

    void doDisplayReleasedEnabled() override
    {
        GuiPainter::instance().blit(return1Bmp_, absoluteBoundary().minCorner());
    }

private:
    MachGuiReturnToMenuBtn(const MachGuiReturnToMenuBtn&);
    MachGuiReturnToMenuBtn& operator=(const MachGuiReturnToMenuBtn&);
    bool operator==(const MachGuiReturnToMenuBtn&) const;

    // Data members...
    MachInGameScreen* pInGameScreen_;
    GuiBitmap return1Bmp_;
    GuiBitmap return2Bmp_;
};

/*
class MachGuiReturnToMenuBtn : public GuiBitmapButtonWithFilledBorder
// cannonical from revoked
{
public:
    MachGuiReturnToMenuBtn( GuiDisplayable *pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen )
    :   GuiBitmapButtonWithFilledBorder(pParent,
                                        rel,
                                        GuiBorderMetrics(1,1,1),
                                        GuiFilledBorderColours( Gui::Colour(96.0/255.0,108.0/255.0,104.0/255.0),
                                                                Gui::Colour(232.0/255.0,232.0/255.0,232.0/255.0),
                                                                Gui::Colour(62.0/255.0,62.0/255.0,62.0/255.0),
                                                                Gui::RED() ),
                                        Gui::bitmap( SysPathName( "gui/misc/return.bmp" ) ),
                                        Gui::Coord(1,1) ),
        pInGameScreen_( pInGameScreen )
    {}

protected:
    // inherited from GuiIcon...
    virtual void doBeReleased( const GuiMouseEvent& )
    {
        pInGameScreen_->switchToMenus( true );
    }

    virtual void doBeDepressed( const GuiMouseEvent& )
    {}

    virtual void doHandleMouseEnterEvent( const GuiMouseEvent& )
    {
        //Load the resource string
        GuiResourceString prompt( IDS_RETURNTOMENUS );

        //Set the cursor prompt
        pInGameScreen_->cursorPromptText( prompt.asString() );
    }

    virtual void doHandleMouseExitEvent( const GuiMouseEvent& )
    {
        //Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();
    }

private:
    MachGuiReturnToMenuBtn( const MachGuiReturnToMenuBtn& );
    MachGuiReturnToMenuBtn& operator =( const MachGuiReturnToMenuBtn& );
    bool operator ==( const MachGuiReturnToMenuBtn& ) const;

    // Data members...
    MachInGameScreen* pInGameScreen_;
};
*/

class MachGuiControlPanelAddOnImpl
{
public:
    MachGuiControlPanelAddOnImpl();

    GuiBitmap backgroundBmp_;
    MachInGameScreen* pInGameScreen_;
};

MachGuiControlPanelAddOnImpl::MachGuiControlPanelAddOnImpl()
    : backgroundBmp_(MachGui::getScaledImage("gui/misc/camtab.bmp"))
{
    backgroundBmp_.enableColourKeying();
}

#define MachGuiControlPanelAddOnWidth 30
#define MachGuiControlPanelAddOnHeight 66

MachGuiControlPanelAddOn::MachGuiControlPanelAddOn(
    GuiDisplayable* pParent,
    const Gui::Coord& coord,
    MachInGameScreen* pInGameScreen)
    : GuiDisplayable(
        pParent,
        Gui::Box(
            coord,
            Gui::Size(MachGuiControlPanelAddOnWidth, MachGuiControlPanelAddOnHeight) * MachGui::uiScaleFactor()),
        GuiDisplayable::LAYER3)
{
    pImpl_ = new MachGuiControlPanelAddOnImpl();

    CB_DEPIMPL(MachInGameScreen*, pInGameScreen_);

    pInGameScreen_ = pInGameScreen;

    new MachGuiReturnToMenuBtn(this, Gui::Coord(4, 0) * MachGui::uiScaleFactor(), pInGameScreen);
    new MachGuiCameraToggleBtn(this, Gui::Coord(4, 22) * MachGui::uiScaleFactor(), pInGameScreen);

    redrawEveryFrame(true);

    TEST_INVARIANT;
}

MachGuiControlPanelAddOn::~MachGuiControlPanelAddOn()
{
    TEST_INVARIANT;
    delete pImpl_;
}

// virtual
void MachGuiControlPanelAddOn::doDisplay()
{
    CB_DEPIMPL(GuiBitmap, backgroundBmp_);

    GuiPainter::instance().blit(backgroundBmp_, absoluteBoundary().minCorner());
}

void MachGuiControlPanelAddOn::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiControlPanelAddOn& t)
{

    o << "MachGuiControlPanelAddOn " << (void*)&t << " start" << std::endl;
    o << "MachGuiControlPanelAddOn " << (void*)&t << " end" << std::endl;

    return o;
}

/* End CTRLADON.CPP *************************************************/
