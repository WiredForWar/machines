/*
 * C O M P I T E M . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/compitem.hpp"

#include "system/registry.hpp"

#include <stdio.h>

static void idToString(const ItemId& id, std::string* stringId)
{
    PRE(stringId);
    char buffer[20];
    sprintf(buffer, "%u", id);
    *stringId = buffer;
}

MachPhysComplexityItem::MachPhysComplexityItem(const ItemId& id)
    : id_(id)
{
}

MachPhysComplexityBooleanItem::MachPhysComplexityBooleanItem(const ItemId& setId, bool enabled)
    : MachPhysComplexityItem(setId)
{
    std::string strId;
    idToString(id(), &strId);

    int value = enabled ? 1 : 0;
    value = SysRegistry::instance().queryIntegerValue("Options\\Graphics Complexity", strId, value);
    enabled_ = value;
}

void MachPhysComplexityBooleanItem::changeState(bool enabled)
{
    if (enabled_ != enabled)
    {
        enabled_ = enabled;

        // persist new setting
        std::string strId;
        idToString(id(), &strId);
        SysRegistry::instance().setIntegerValue("Options\\Graphics Complexity", strId, enabled_);
    }
}

MachPhysComplexityChoiceItem::MachPhysComplexityChoiceItem(const ItemId& setId, uint nChoices, uint choice)
    : MachPhysComplexityItem(setId)
    , nChoices_(nChoices)
    , choice_(0)
{
    std::string strId;
    idToString(id(), &strId);

    choice = SysRegistry::instance().queryIntegerValue("Options\\Graphics Complexity", strId, choice);

    PRE(choice < nChoices);
    choice_ = choice;
}

uint MachPhysComplexityChoiceItem::nChoices() const
{
    return nChoices_;
}

uint MachPhysComplexityChoiceItem::choice() const
{
    return choice_;
}

void MachPhysComplexityChoiceItem::changeState(uint choice)
{
    PRE(choice < nChoices_);
    if (choice_ != choice)
    {
        choice_ = choice;

        // persist new setting
        std::string strId;
        idToString(id(), &strId);
        SysRegistry::instance().setIntegerValue("Options\\Graphics Complexity", strId, choice_);
    }
}

/* End COMPITEM.CPP **************************************************/
