/*
 * A N I M A T I O . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiAnimation

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_ANIMATIO_HPP
#define _MACHGUI_ANIMATIO_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "gui/displaya.hpp"
#include "gui/gui.hpp"
#include "ctl/pvector.hpp"

class SysPathName;

class MachGuiAnimation : public GuiDisplayable
// Canonical form revoked
{
public:
    struct Cell
    {
        Cell();

        enum CellTime
        {
            STATIC,
            RANDOM
        };
        GuiBitmap cell_;
        double finishTime_;
        double duration_;
        double randomDurationMinTime_;
        double randomDurationMaxTime_;
        CellTime cellTimeType_;
        bool hasSound_;
        string wavFile_;
    };

    using Cells = ctl_pvector<Cell>;

    // Create animation from file describing animation.
    // - optional colourKey argument is used to switch colourKeying on for each frame of the animation.
    // - xOffset and yOffset arguments will change the x/y values specified in the .anm file.
    static MachGuiAnimation* createAnimation(
        GuiDisplayable* pParent,
        const SysPathName& animPath,
        bool colourKey = false,
        int xOffset = 0,
        int yOffset = 0);

    // MachGuiAnimationCells* must be newed and will be deleted by this class.
    MachGuiAnimation(GuiDisplayable* pParent, const Gui::Box&, Cells*);
    ~MachGuiAnimation() override;

    void CLASS_INVARIANT;

    void update();

    // Force the animation to jump to a particular cell (frame).
    void jumpToCell(size_t cellIndex);
    // PRE( cellIndex < pCells_->size() );

    // Return the cell that the animation is currently on.
    size_t cellIndex() const;

    // Get the number of cells in the animation.
    size_t numCells() const;

protected:
    void doDisplay() override;

    void updateCellTimes();
    void playSound(const string& wavFile);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiAnimation& t);

    MachGuiAnimation(const MachGuiAnimation&);
    MachGuiAnimation& operator=(const MachGuiAnimation&);

    // Data members...
    Cells* pCells_;
    double startTime_;
    size_t cellIndex_;
    size_t loopBack_;
};

class MachGuiAnimations
{
public:
    MachGuiAnimations(GuiDisplayable*, const SysPathName& pathName);
    ~MachGuiAnimations();

    void update();

private:
    ctl_pvector<MachGuiAnimation> animations_;
};

#endif

/* End ANIMATIO.HPP *************************************************/
