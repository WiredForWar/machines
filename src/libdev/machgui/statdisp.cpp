/*
 * S T A T D I S P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/statdisp.hpp"

#include "machgui/gui.hpp"
#include "machgui/statbar.hpp"
#include "machgui/ui/MenuText.hpp"

#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "machphys/machphys.hpp"
#include "system/pathname.hpp"

float MachGuiStatisticsDisplay::initialTime_ = 0;
float MachGuiStatisticsDisplay::ratioComplete_ = 0;
float MachGuiStatisticsDisplay::rampDuration_ = 1.0;

MachGuiStatisticsDisplay::MachGuiStatisticsDisplay(
    GuiDisplayable* pParent,
    const Gui::Coord& topLeft,
    MachPhys::Race race)
    : GuiDisplayable(pParent, Gui::Boundary(topLeft, MexSize2d(75, 42) * MachGui::menuScaleFactor()))
    , topLeft_(topLeft)
    , redraw_(true)
{
    SysPathName backgroundBmpFile("gui/menu/barback.bmp");
    ASSERT(backgroundBmpFile.insensitiveExistsAsFile(), backgroundBmpFile.c_str());

    backgroundBmp_ = Gui::requestScaledImage(backgroundBmpFile.pathname(), MachGui::menuScaleFactor());

    string bar1, bar2, bar3, bar4;
    switch (race)
    {
        case MachPhys::N_RACES:
            bar1 = "gui/menu/redbar.bmp";
            bar2 = "gui/menu/bluebar.bmp";
            bar3 = "gui/menu/greenbar.bmp";
            bar4 = "gui/menu/yellobar.bmp";
            break;
        case MachPhys::RED:
            bar1 = bar2 = bar3 = bar4 = "gui/menu/redbar.bmp";
            break;
        case MachPhys::YELLOW:
            bar1 = bar2 = bar3 = bar4 = "gui/menu/yellobar.bmp";
            break;
        case MachPhys::BLUE:
            bar1 = bar2 = bar3 = bar4 = "gui/menu/bluebar.bmp";
            break;
        case MachPhys::GREEN:
            bar1 = bar2 = bar3 = bar4 = "gui/menu/greenbar.bmp";
            break;
    }
    uint x = 33 * MachGui::menuScaleFactor();
    uint y = 3 * MachGui::menuScaleFactor(); // relative to top left of stats display background
    statBars_.push_back(new MachGuiStatisticsBar(this, Gui::Coord(x, y), bar1));
    uint height = (*statBars_.begin())->height() + 2 * MachGui::menuScaleFactor();
    statBars_.push_back(new MachGuiStatisticsBar(this, Gui::Coord(x, y += height), bar2));
    statBars_.push_back(new MachGuiStatisticsBar(this, Gui::Coord(x, y += height), bar3));
    statBars_.push_back(new MachGuiStatisticsBar(this, Gui::Coord(x, y += height), bar4));

    stats_.push_back(0);
    stats_.push_back(0);
    stats_.push_back(0);
    stats_.push_back(0);

    TEST_INVARIANT;
}

MachGuiStatisticsDisplay::~MachGuiStatisticsDisplay()
{
    for (StatBars::iterator i = statBars_.begin(); i != statBars_.end(); i++)
        delete *i;

    TEST_INVARIANT;
}

void MachGuiStatisticsDisplay::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void MachGuiStatisticsDisplay::setStatistics(int stat1, int stat2, int stat3, int stat4)
{
    stats_[0] = stat1;
    stats_[1] = stat2;
    stats_[2] = stat3;
    stats_[3] = stat4;

    // find largest statistic
    // kludge - should be 0, but would mess up range precondition
    int maxStatistic = 1;
    for (Stats::iterator stat = stats_.begin(); stat != stats_.end(); ++stat)
    {
        if (*stat > maxStatistic)
            maxStatistic = *stat;
    }
    // set range in terms of highest statistic
    for (StatBars::iterator bar = statBars_.begin(); bar != statBars_.end(); ++bar)
    {
        (*bar)->range(maxStatistic);
    }
}

void MachGuiStatisticsDisplay::update()
{
    // Force object to be redrawn ( will indirectly call doDisplay() )
    changed();
}

// virtual
void MachGuiStatisticsDisplay::doDisplay()
{
    PRE(statBars_.size() == stats_.size());

    if (redraw())
    {
        // Draw statistics bars on background bitmap, ramping up bars until actual value is reached
        if (backgroundBmp_.requestedSize().isNull())
        {
            GuiPainter::instance().blit(backgroundBmp_, absoluteBoundary().minCorner());
        }
        else
        {
            Gui::Size backgroundSize(backgroundBmp_.requestedSize().width, backgroundBmp_.requestedSize().height);
            GuiPainter::instance().stretch(backgroundBmp_, Gui::Box(absoluteBoundary().minCorner(), backgroundSize));
        }

        bool redraw = false;
        for (uint i = 0; i < statBars_.size(); ++i)
        {
            if (statBars_[i]->stat() < stats_[i])
            {
                float newStat = ratioComplete_ * stats_[i];
                // Check for rounding errors
                if (newStat > statBars_[i]->range())
                {
                    newStat = statBars_[i]->range();
                }
                statBars_[i]->stat(newStat);
                redraw = true;
            }
            // Special case - -ve numbers: we want to set the statistic to be -ve,
            // but not do the ramping
            else if (stats_[i] < 0)
            {
                statBars_[i]->stat(stats_[i]);
            }
        }
        for (StatBars::iterator bar = statBars_.begin(); bar != statBars_.end(); ++bar)
        {
            (*bar)->draw(redraw);
        }
        //      redraw_ = redraw;
        redraw_ = true; // Always redraw
    }
}

// static
void MachGuiStatisticsDisplay::setTime(double newTime)
{
    float interval = newTime - initialTime_;
    ratioComplete_ = interval / rampDuration_;
}

std::ostream& operator<<(std::ostream& o, const MachGuiStatisticsDisplay& t)
{

    o << "MachGuiStatisticsDisplay " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiStatisticsDisplay " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End STATDISP.CPP *************************************************/
