/*
 * L I S T O B V R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "gui/listobvr.hpp"
#include "gui/scrolist.hpp"

GuiListObserver::GuiListObserver(GuiSimpleScrollableList* pList)
    : pList_(pList)
{
    PRE(pList);

    list().notifyMe(this);

    TEST_INVARIANT;
}

// virtual
GuiListObserver::~GuiListObserver()
{
    TEST_INVARIANT;

    if (hasList())
    {
        list().dontNotifyMe(this);
    }
}

void GuiListObserver::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const GuiListObserver& t)
{

    o << "GuiListObserver " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "GuiListObserver " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void GuiListObserver::listDeleted()
{
    pList_ = nullptr;
}

bool GuiListObserver::hasList() const
{
    return pList_ != nullptr;
}

GuiSimpleScrollableList& GuiListObserver::list()
{
    PRE(hasList());

    return *pList_;
}

const GuiSimpleScrollableList& GuiListObserver::list() const
{
    PRE(hasList());

    return *pList_;
}

/* End LISTOBVR.CPP *************************************************/
