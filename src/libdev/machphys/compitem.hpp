/*
 * C O M P I T E M . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysComplexityItem

    A brief description of the class should go in here
*/

#ifndef _COMPITEM_HPP
#define _COMPITEM_HPP

#include "base/base.hpp"

#include <functional>

using ItemId = uint;

class MachPhysComplexityItem
// Canonical form revoked
{
public:
    using Callback = std::function<void(MachPhysComplexityItem*)>;

    MachPhysComplexityItem(const ItemId& id);

    const ItemId& id() const { return id_; }

    void setCallback(Callback callback) { callback_ = callback; }

    void update()
    {
        if (!callback_)
            return;

        callback_(this);
    }

private:
    friend ostream& operator<<(ostream& o, const MachPhysComplexityItem& t);

    MachPhysComplexityItem();
    MachPhysComplexityItem(const MachPhysComplexityItem&);
    MachPhysComplexityItem& operator=(const MachPhysComplexityItem&);

protected:
    ItemId id_;
    Callback callback_;
};

class MachPhysComplexityBooleanItem : public MachPhysComplexityItem
{
public:
    MachPhysComplexityBooleanItem(const ItemId& id, bool enabled);

    bool enabled() const { return enabled_; }
    void changeState(bool enabled);

protected:
    bool enabled_ = false;
};

class MachPhysComplexityChoiceItem : public MachPhysComplexityItem
{
public:
    MachPhysComplexityChoiceItem(const ItemId& id, uint nChoices, uint choice = 0);

    uint nChoices() const;
    uint choice() const;
    void changeState(uint choice);

protected:
    uint nChoices_;
    uint choice_;
};

#endif

/* End COMPITEM.HPP **************************************************/
