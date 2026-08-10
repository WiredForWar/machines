/*
 * P L A Y S C R I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiPlayerScore

    A brief description of the class should go in here
*/

#ifndef _PLAYSCRI_HPP
#define _PLAYSCRI_HPP

#include "base/base.hpp"
#include "machgui/Statistics.hpp"
#include "machgui/internal/PlayerNameList.hpp"
#include "machgui/StatisticsDisplay.hpp"

class MachGuiPlayerScore
    : public MachGuiStatistics
    , public MachGuiPlayerNameList
// Canonical form revoked
{
public:
    MachGuiPlayerScore(GuiDisplayable* pParent, const Gui::Coord& topLeft);
    ~MachGuiPlayerScore() override;

    void CLASS_INVARIANT;

    void update() override;

    void setStatistics(int, int, int, int) override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiPlayerScore& t);

    MachGuiPlayerScore(const MachGuiPlayerScore&) = delete;
    MachGuiPlayerScore& operator=(const MachGuiPlayerScore&) = delete;

    MachGuiStatisticsDisplay statDisplay_;
};

#endif

/* End PLAYSCRI.HPP *************************************************/
