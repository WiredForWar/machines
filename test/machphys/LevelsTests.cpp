#include <gtest/gtest.h>

#include "machphys/Data/Levels.hpp"
#include "machphys/machphys.hpp"

#include <initializer_list>

///////////////////////////////////////////////////////////////////////////////

// Hardware and software levels are two separate lists, and a machine exists
// only at a pairing taken from both of them. Anything holding one level and
// having to supply the other cannot assume the software level is 1: it is 1 for
// everything that walks around except two of the three technicians.

namespace
{

// Whoever has to supply a software level and has only been given a hardware one
// takes the lowest the machine comes in. That is only a safe thing to take for
// as long as it goes with every hardware level the same machine comes in, so
// that is what is checked here rather than any particular pair.
template <typename SubType> void expectLowestSoftwareLevelPairsWithEveryHardwareLevel(SubType subType)
{
    const MachPhysLevels& levels = MachPhysLevels::instance();

    ASSERT_GT(levels.nSoftwareLevels(subType), 0u);
    const size_t lowestSoftware = levels.softwareLevel(subType, 0);

    ASSERT_GT(levels.nHardwareLevels(subType), 0u);
    for (size_t i = 0; i != levels.nHardwareLevels(subType); ++i)
    {
        const size_t hardware = levels.hardwareLevel(subType, i);
        EXPECT_TRUE(levels.levelValid(subType, hardware, lowestSoftware))
            << "hardware level " << hardware << " does not go with software level " << lowestSoftware;
    }
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(MachPhysLevelsTests, ATechBoyAndABrainBoxDoNotExistAtSoftwareLevelOne)
{
    const MachPhysLevels& levels = MachPhysLevels::instance();

    // Software level 1 is the obvious thing to assume and it is wrong for two
    // technicians out of three. Assuming it does not build a poorer machine, it
    // asks for machine data that was never read.
    EXPECT_TRUE(levels.levelValid(MachPhys::LAB_TECH, 2, 1));
    EXPECT_FALSE(levels.levelValid(MachPhys::TECH_BOY, 3, 1));
    EXPECT_FALSE(levels.levelValid(MachPhys::BRAIN_BOX, 5, 1));
}

TEST(MachPhysLevelsTests, TechniciansComeAtThePairingsTheScenariosSpawnThemAt)
{
    const MachPhysLevels& levels = MachPhysLevels::instance();

    // The three pairings the shipped .scn files use, and the only ones there
    // are: a technician's hardware level settles its software level.
    EXPECT_TRUE(levels.levelValid(MachPhys::LAB_TECH, 2, 1));
    EXPECT_TRUE(levels.levelValid(MachPhys::TECH_BOY, 3, 3));
    EXPECT_TRUE(levels.levelValid(MachPhys::BRAIN_BOX, 5, 5));
}

TEST(MachPhysLevelsTests, TheLowestSoftwareLevelOfASubtypedMachineGoesWithAllItsHardware)
{
    for (MachPhys::TechnicianSubType subType : { MachPhys::LAB_TECH, MachPhys::TECH_BOY, MachPhys::BRAIN_BOX })
        expectLowestSoftwareLevelPairsWithEveryHardwareLevel(subType);

    for (MachPhys::AggressorSubType subType :
         { MachPhys::GRUNT, MachPhys::ASSASSIN, MachPhys::KNIGHT, MachPhys::BALLISTA, MachPhys::NINJA })
        expectLowestSoftwareLevelPairsWithEveryHardwareLevel(subType);

    for (MachPhys::AdministratorSubType subType : { MachPhys::BOSS, MachPhys::OVERSEER, MachPhys::COMMANDER })
        expectLowestSoftwareLevelPairsWithEveryHardwareLevel(subType);

    for (MachPhys::ConstructorSubType subType : { MachPhys::DOZER, MachPhys::BUILDER, MachPhys::BEHEMOTH })
        expectLowestSoftwareLevelPairsWithEveryHardwareLevel(subType);
}

TEST(MachPhysLevelsTests, TheLowestSoftwareLevelOfAPlainMachineGoesWithAllItsHardware)
{
    // The machines that come in one flavour each, which are the only types the
    // levels take without a subtype.
    for (MachPhys::MachineType type :
         { MachPhys::GEO_LOCATOR, MachPhys::SPY_LOCATOR, MachPhys::RESOURCE_CARRIER, MachPhys::APC })
        expectLowestSoftwareLevelPairsWithEveryHardwareLevel(type);
}
