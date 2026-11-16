#include "gui/gui.hpp"
#include "machgui/NavigatorBase.hpp"
#include "machgui/SquadronBank.hpp"

#include <gtest/gtest.h>

class HudLayoutTests : public ::testing::Test
{
protected:
    void TearDown() override { Gui::setUiScaleFactor(1); }
};

TEST_F(HudLayoutTests, TheSquadronBankFitsTheNavigatorSpaceAtEverySupportedScale)
{
    for (int scale = 1; scale <= 4; ++scale)
    {
        SCOPED_TRACE(scale);
        Gui::setUiScaleFactor(scale);
        EXPECT_LE(MachGuiSquadronBank::reqHeight(), MachGuiNavigatorBase::reqHeight());
    }
}

TEST_F(HudLayoutTests, EnlargingTheBankPreservesTheOriginalRowOverlap)
{
    Gui::setUiScaleFactor(1);
    const auto originalHeight = MachGuiSquadronBank::reqHeight();
    for (int scale = 2; scale <= 4; ++scale)
    {
        SCOPED_TRACE(scale);
        Gui::setUiScaleFactor(scale);
        EXPECT_EQ(originalHeight * scale, MachGuiSquadronBank::reqHeight());
    }
}
