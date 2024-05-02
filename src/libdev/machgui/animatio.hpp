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

#include <memory>

class SysPathName;

class MachGuiAnimation : public GuiDisplayable
// Canonical form revoked
{
public:
    struct Cell
    {
        enum CellTime
        {
            STATIC,
            RANDOM
        };
        GuiBitmap cell_{};
        double finishTime_{};
        double duration_{};
        double randomDurationMinTime_{};
        double randomDurationMaxTime_{};
        CellTime cellTimeType_{};
        bool hasSound_{};
        std::string wavFile_;
    };

    using Cells = std::vector<Cell>;

    // Create animation from file describing animation.
    // - optional colourKey argument is used to switch colourKeying on for each frame of the animation.
    // - xOffset and yOffset arguments will change the x/y values specified in the .anm file.
    static MachGuiAnimation* createAnimation(
        GuiDisplayable* pParent,
        const SysPathName& animPath,
        bool colourKey = false,
        int xOffset = 0,
        int yOffset = 0,
        float scale = 1);

    MachGuiAnimation(GuiDisplayable* pParent, const Gui::Box&, std::unique_ptr<Cells> pCells);

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
    void playSound(const std::string& wavFile);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiAnimation& t);

    MachGuiAnimation(const MachGuiAnimation&);
    MachGuiAnimation& operator=(const MachGuiAnimation&);

    // Data members...
    std::unique_ptr<Cells> pCells_;
    double startTime_;
    size_t cellIndex_;
    size_t loopBack_;
};

class MachGuiAnimations
{
public:
    void init(GuiDisplayable* pParent, const SysPathName& pathName, float scale);
    void update();

private:
    ctl_pvector<MachGuiAnimation> animations_;
};

#endif

/* End ANIMATIO.HPP *************************************************/
