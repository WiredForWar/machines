/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "ctl/vector.hpp"
#include "machgui/HWResearchIcon.hpp"
#include "machgui/HWResearchBank.hpp"
#include "machgui/ActorBitmaps.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/ActorStringIdRestorer.hpp"
#include "machlog/Actors/Actor.hpp"
#include "machlog/Actors/HardwareLab.hpp"
#include "machlog/Tech/ResearchItem.hpp"
#include "machlog/Races.hpp"
#include "gui/font.hpp"
#include "gui/restring.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/internal/SoundManager.hpp"

/* ////////////////////////////////////////////// constructor /////////////////////////////////////////////////// */

MachHWResearchIcon::MachHWResearchIcon(
    GuiDisplayable* pParent,
    MachInGameScreen* pInGameScreen,
    MachHWResearchBank* pHWResearchBank,
    MachLogHardwareLab* pHardwareLab,
    const MachLogResearchItem* pResearchItem)
    : GuiIcon(
        pParent,
        Gui::Coord(0, 0), // Will be relocated by icon sequence parent
        MachGui::getScaledImage(MachActorBitmaps::name(
            pResearchItem->objectType(),
            pResearchItem->subType(),
            pResearchItem->hwLevel(),
            pResearchItem->weaponCombo(),
            MachLogRaces::instance().playerRace())))
    , pInGameScreen_(pInGameScreen)
    , pHWResearchBank_(pHWResearchBank)
    , pHardwareLab_(pHardwareLab)
    , pResearchItem_(pResearchItem)
{

    TEST_INVARIANT;
}

/* /////////////////////////////////////////////// destructor /////////////////////////////////////////////////// */

MachHWResearchIcon::~MachHWResearchIcon()
{
    TEST_INVARIANT;
}

std::string MachHWResearchIcon::getPromptText() const
{
    GuiString prompt = MachLogActorStringIdRestorer::getActorPromptText(
        pResearchItem_->objectType(),
        pResearchItem_->subType(),
        pResearchItem_->weaponCombo(),
        pResearchItem_->hwLevel(),
        IDS_RESEARCH_PROMPT,
        IDS_RESEARCH_WITH_WEAPON_PROMPT);

    // Add bmu cost and rp cost to end of prompt text
    char bmuBuffer[20];
    char rpBuffer[20];
    snprintf(bmuBuffer, sizeof(bmuBuffer), "%c%d", GuiBmpFont::bmuPointsIndex(), pResearchItem_->buildingCost());
    snprintf(rpBuffer, sizeof(rpBuffer), "%c%d", GuiBmpFont::researchPointsIndex(), pResearchItem_->researchCost());

    if (pResearchItem_->buildingCost() != 0)
    {
        GuiStrings strings;
        strings.push_back(GuiString(bmuBuffer));
        strings.push_back(GuiString(rpBuffer));
        GuiResourceString costText(IDS_COST_WITH_RP, strings);
        prompt += "\n" + costText.asString();
    }
    else
    {
        GuiResourceString costText(IDS_COST, GuiString(rpBuffer));
        prompt += "\n" + costText.asString();
    }

    return prompt;
}

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchIcon::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachHWResearchIcon& t)
{

    o << "MachHWResearchIcon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachHWResearchIcon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doBeDepressed(const GuiMouseEvent&)
{
    MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doBeReleased(const GuiMouseEvent&)
{
    // Add the item to the queue
    if (pHardwareLab_->addResearchItem(*pResearchItem_))
    {
        // Wasn't in the queue previously, so update the bank icons
        pHWResearchBank_->updateQueueIcons();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchIcon::reqWidth()
{
    return 42 * Gui::uiScaleFactor(); // Todo : remove hardcoding
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchIcon::reqHeight()
{
    return 42 * Gui::uiScaleFactor(); // todo : remove hardcoding
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent)
{
    GuiIcon::doHandleMouseEnterEvent(mouseEvent);

    pInGameScreen_->setCursorPromptText(getPromptText());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent)
{
    // Clear the cursor prompt string
    pInGameScreen_->clearCursorPromptText();

    GuiIcon::doHandleMouseExitEvent(mouseEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* End *************************************************/
