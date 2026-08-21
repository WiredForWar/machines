/*
 * D I S P L A Y . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _REN_DISPLAY_HPP
#define _REN_DISPLAY_HPP

#include "base/base.hpp"
#include "render/DisplayModeCatalogue.hpp"
#include "render/IWindowAdapter.hpp"
#include "render/render.hpp"

#include <vector>

template <class T> class ctl_list;
class SysPathName;
class RenSurface;
class RenCursor2d;
class RenIDisplay;

// When an app goes into full-screen exclusive mode, this class
// is used to set the mode.
class RenDisplay
{
public:
    // The display does NOT own the adapter; the caller manages its lifetime.
    explicit RenDisplay(Ren::IWindowAdapter* adapter);
    ~RenDisplay();

    using WindowMode = Ren::WindowMode;

    // Ask for the way the window covers the display. Returns whether it ended up in
    // the mode asked for; windowMode() reports what was settled on either way.
    //
    // The mode chosen decides how much of a display mode is the caller's to pick,
    // so set it before choosing one with useMode().
    bool useWindowMode(WindowMode mode);
    WindowMode windowMode() const;

    class Mode
    {
    public:
        Mode() = default;
        Mode(const Mode& m) = default;

        Mode& operator=(const Mode& m)
        {
            width_ = m.width_;
            height_ = m.height_;
            depth_ = m.depth_;
            rate_ = m.rate_;
            format_ = m.format_;
            return *this;
        }
        bool operator==(const Mode& m) const
        {
            return width_ == m.width_ && height_ == m.height_ && (rate_ == m.rate_ || rate_ == 0 || m.rate_ == 0);
        }

        bool isValid() const { return width_; }

        int width() const { return width_; }
        int height() const { return height_; }
        Ren::Size size() const { return { width_, height_ }; }
        int bitDepth() const { return depth_; }
        int refreshRate() const { return rate_; }
        int pixels() const { return width_ * height_; }
        int memoryRequired() const { return (pixels() * depth_) / 8; }

    private:
        // Only RenDisplay can create modes.  Thus, clients are prevented from
        // requesting modes that aren't actually possible.
        friend class RenDisplay;
        friend class RenIDisplay;
        friend class RenDDEnumerator;
        Mode(int w, int h, int r)
            : width_(w)
            , height_(h)
            , depth_(32)
            , rate_(r)
            , format_(0)
        {
        }

        Mode(int w, int h, int d, int r, uint32_t format)
            : width_(w)
            , height_(h)
            , depth_(d)
            , rate_(r)
            , format_(format)
        {
        }
        int width_{};
        int height_{};
        int depth_{};
        int rate_{};
        uint32_t format_{};
    };

    // Clients cannot create modes, they must use modeList to get
    // a list of modes, choose one and pass a reference to useMode.
    using Modes = std::vector<Mode>;
    const Modes& modeList() const;

    // The same modes as a list of resolutions and the rates each is offered at,
    // for offering the player a choice. Built with the mode list.
    const Ren::DisplayModeCatalogue& modeCatalogue() const;

    bool useMode(const Mode&);
    const Mode& currentMode() const;
    void buildDisplayModesList();
    const Mode getDesktopDisplayMode() const;
    const Mode getFailSafeDisplayMode() const;

    const Mode findMode(int width, int height, int refreshRate);
    const Mode getWindowedMode(int width, int height) const;

    const Mode& lowestAllowedMode() const;
    bool lowestAllowedMode(const Mode& lowMode);
    bool lowestAllowedMode(int width, int height, int depth);

    const Mode& highestAllowedMode() const;
    bool setHighestAllowedMode(uint32_t maxMemory);
    bool setHighestAllowedMode();

    // If there's not enough video memory to create all the required
    // surfaces at the current resolution, this method can be used to
    // switch to a lower resolution, at the *same* bit depth.  Returns
    // false if there isn't a lower mode available.
    // PRE(modeList().size() > 0);
    bool useLowerMode();

    // Finds the nearest mode for which Mode::pixels() <= pixels and
    // for which the depth is an *exact* match.  Returns false if there
    // was no suitable mode.
    // PRE(modeList().size() > 0);
    bool useNearestMode(int pixels, int bitDepth);

    // Finds the lowest resolution mode for the given depth.  Returns false
    // if there was no mode of the given depth.  If no depth is specified,
    // you get the lowest mode.
    // PRE(modeList().size() > 0);
    bool useLowestMode(int bitDepth);

    enum MemoryType
    {
        SYSTEM_MEMORY,
        VIDEO_MEMORY
    };

    // Call when rendering is finished to swap the front and back buffers.
    // If zbDepth==0 no z-buffer is created.
    bool createSurfaces(MemoryType zbType, int zbDepth);

    // Updates the cursor, if necessary.
    void startFrame();

    void flipBuffers();

    // This method should be called every frame in case Alt-Tab has been
    // pressed.  Returns false if one of the buffers is lost and it could
    // not be restored.  Rendering should not proceed if this returns false.
    bool checkForLostSurfaces();

    // Beware: if this display releases the front buffer, back buffer
    // pointers may become invalid.  There's not much that can be done
    // about this, apart from writing a very sophisticated version of
    // COMPtr which knows about the dependencies.
    void useCursor(const RenCursor2d*);
    const RenCursor2d* currentCursor() const;

    // Whether the cursor is drawn into the frame. A cursor that is not drawn is
    // still a cursor: it keeps being pointed at things and keeps taking clicks,
    // it just leaves no arrow in the picture. For photographing what is behind
    // it.
    void setCursorVisible(bool visible);
    bool isCursorVisible() const;

    void setCursorGrabEnabled(bool enabled);

    // Ask the system to report pointer motion relative to where the pointer already is,
    // instead of moving a visible pointer around the window. While enabled the pointer
    // is hidden and its position stops changing, so motion can be reported without
    // being limited by the edges of the window and without the system's pointer
    // acceleration applied. Returns false if the system refused, in which case the
    // pointer keeps behaving as it did.
    bool setRelativeMouseModeEnabled(bool enabled);

    void supportsGammaCorrection(bool);
    bool supportsGammaCorrection() const;
    void gammaCorrection(const double& gammaCorrection);
    const double& gammaCorrection() const;

    bool isPrimaryDriver() const;

    uint32_t frameNumber() const;

    Ren::IWindowAdapter* adapter();
    const Ren::IWindowAdapter* adapter() const;

    RenIDisplay& displayImpl();
    const RenIDisplay& displayImpl() const;
    // PRE(pImpl_!=NULL);

    void CLASS_INVARIANT;

private:
    // **Important** the back buffer must be released before the primary.  The
    // order of declartion is critical.  See note in fallibleCreateSurfaces.
    RenIDisplay* pImpl_;

    bool fallibleCreateSurfaces(MemoryType zbType, int zbDepth);

    friend class RenDDEnumerator;

    // Operations deliberately revoked
    RenDisplay(const RenDisplay&);
    RenDisplay& operator=(const RenDisplay&);
    bool operator==(const RenDisplay&);
};

std::ostream& operator<<(std::ostream& o, const RenDisplay& t);
std::ostream& operator<<(std::ostream& o, const RenDisplay::Mode& t);

bool operator<(const RenDisplay::Mode& mode1, const RenDisplay::Mode& mode2);
bool operator>(const RenDisplay::Mode& mode1, const RenDisplay::Mode& mode2);
bool operator>=(const RenDisplay::Mode& mode1, const RenDisplay::Mode& mode2);

#endif

/* End DISPLAY.HPP **************************************************/
