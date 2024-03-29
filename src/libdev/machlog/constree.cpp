/*
 * C O N S T R E E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "machlog/mlnotif.hpp"
#include "machlog/consitem.hpp"
#include "machlog/constree.hpp"
#include "utility/linetok.hpp"
#include "machlog/scenario.hpp"
#include "machlog/consitem.hpp"
#include "machlog/mlnotif.hpp"
#include "machlog/vmman.hpp"
#include "machlog/vmdata.hpp"
#include "machlog/internal/constrei.hpp"
#include "machphys/symparse.hpp"

#include "system/metafile.hpp"
#include "system/metaistr.hpp"
#include <memory>

/* //////////////////////////////////////////////////////////////////////////// */

MachLogConstructionTree::MachLogConstructionTree()
    : pImpl_(new MachLogConstructionTreeImpl())
{
    CB_MachLogConstructionTree_DEPIMPL();

    nNotifiables_ = 0;
    constructionItems_.reserve(50);
    notifiables_.reserve(10);
    SysPathName globalTree("data/global.cst");
    readAllItems(globalTree);
    TEST_INVARIANT;
}

/* //////////////////////////////////////////////////////////////////////////// */

MachLogConstructionTree::~MachLogConstructionTree()
{
    CB_MachLogConstructionTree_DEPIMPL();

    TEST_INVARIANT;
    while (constructionItems_.size() > 0)
    {
        delete constructionItems_.front();
        constructionItems_.erase(constructionItems_.begin());
    }

    delete pImpl_;
}

/* //////////////////////////////////////////////////////////////////////////// */

const MachLogConstructionTree::ConstructionItems& MachLogConstructionTree::constructionItems() const
{
    CB_MachLogConstructionTree_DEPIMPL();

    return constructionItems_;
}

//////////////////////////////////////////////////////////////////////////////

MachLogConstructionTree::ConstructionItems& MachLogConstructionTree::constructionItems()
{
    CB_MachLogConstructionTree_DEPIMPL();

    return constructionItems_;
}

//////////////////////////////////////////////////////////////////////////////

bool MachLogConstructionTree::activated(
    MachPhys::Race r,
    MachLog::ObjectType constructionType,
    int subType,
    size_t hwLevel,
    MachPhys::WeaponCombo wc) const
{
    CB_MachLogConstructionTree_DEPIMPL();

    bool isActivated = false;
    ConstructionItems::iterator i = constructionItems_.begin();
    for (; i != constructionItems_.end() && ! isActivated; ++i)
    {
        if (constructionType == (*i)->constructionType() && (*i)->activated(r))
        {
            if (subType == MachLog::DONTCARE)
            {
                ASSERT(
                    hwLevel == MachLog::DONTCARE && wc == MachPhys::N_WEAPON_COMBOS,
                    "No further parameters should be passed after a DONTCARE parameter.");
                isActivated = true;
            }

            else if (subType == (*i)->subType())
            {
                if (hwLevel == MachLog::DONTCARE)
                {
                    ASSERT(
                        wc == MachPhys::N_WEAPON_COMBOS,
                        "No further parameters should be passed after a DONTCARE parameter.");
                    isActivated = true;
                }

                else if (hwLevel == (*i)->hwLevel())

                {
                    if (wc == MachPhys::N_WEAPON_COMBOS || wc == (*i)->weaponCombo())
                        isActivated = true;
                }
            }
        }
    }

    TEST_INVARIANT;

    return isActivated;
}

//////////////////////////////////////////////////////////////////////////////

MachLogConstructionItem& MachLogConstructionTree::constructionItem(
    MachPhys::ConstructionType constructionType,
    int subType,
    size_t hwLevel,
    MachPhys::WeaponCombo wc)
{
    CB_MachLogConstructionTree_DEPIMPL();

    bool found = false;
    MachLogConstructionItem* pRI = nullptr;
    ConstructionItems::iterator i = constructionItems_.begin();
    for (; i != constructionItems_.end() && ! found; ++i)
    {
        MachLogConstructionItem* pCandidateItem = (*i);
        if (pCandidateItem->constructionType() == constructionType && pCandidateItem->subType() == subType
            && pCandidateItem->hwLevel() == hwLevel && pCandidateItem->weaponCombo() == wc)
        {
            pRI = pCandidateItem;
            found = true;
        }
    }
    ASSERT_INFO(constructionType);
    ASSERT_INFO(subType);
    ASSERT_INFO(hwLevel);
    ASSERT_INFO(wc);
    ASSERT(found, "Didn't find combination in construction tree.\n");

    TEST_INVARIANT;

    return *pRI;
}

//////////////////////////////////////////////////////////////////////////////

void MachLogConstructionTree::updateAvailableConstructions(
    MachPhys::Race r,
    MachPhys::ConstructionType constructionType,
    int subType,
    size_t hwLevel,
    MachPhys::WeaponCombo wc)
{
    CB_MachLogConstructionTree_DEPIMPL();

    // Call sister function with second argument converted from MachPhys::ConstructionType to MachLog::ObjectType.
    updateAvailableConstructions(
        r,
        MachLogScenario::physConstructionToLogObject(constructionType),
        subType,
        hwLevel,
        wc);
}

//////////////////////////////////////////////////////////////////////////////

void MachLogConstructionTree::updateAvailableConstructions(
    MachPhys::Race r,
    MachLog::ObjectType constructionType,
    int subType,
    size_t hwLevel,
    MachPhys::WeaponCombo wc)
{
    CB_MachLogConstructionTree_DEPIMPL();

    bool found = false;

    MachLogConstructionItem* pItemSpecified = nullptr;

    for (ConstructionItems::iterator i = constructionItems_.begin(); i != constructionItems_.end() && ! found; ++i)
    {
        MachLogConstructionItem* pCandidateItem = (*i);
        if (pCandidateItem->constructionType() == constructionType && pCandidateItem->subType() == subType
            && pCandidateItem->hwLevel() == hwLevel && pCandidateItem->weaponCombo() == wc)
        {
            pItemSpecified = pCandidateItem;
            found = true;
            break;
        }
    }
    ASSERT_INFO(constructionType);
    ASSERT_INFO(subType);
    ASSERT_INFO(hwLevel);
    ASSERT_INFO(wc);
    ASSERT(found, "updateAvailableConstructions: Didn't find combination in construction tree.\n");

    // Okay, we found the construction item in question. Now we simply make activate ALL items preceding it in the list
    // (for this race).

    if (! pItemSpecified->activated(r))
    // (no point updating if this entry point is already activated for the race - everything before will be, as well,
    //  should that be the case.)
    {
        bool nothingNewActivated = true;

        for (ConstructionItems::iterator i = constructionItems_.begin(); i != constructionItems_.end(); ++i)
        {
            MachLogConstructionItem* pCandidateItem = (*i);
            if (! pCandidateItem->activated(r) && ! pCandidateItem->activationLocked(r))
            {
                pCandidateItem->activate(r);
                nothingNewActivated = false;
            }

            if (pCandidateItem == pItemSpecified)
            {
                break;
            }
        }

        if (! nothingNewActivated)
        {
            // send a voicemail to the race who now has new constructions online
            MachLogVoiceMailManager::instance().postNewMail(VID_POD_NEW_CONSTRUCTIONS, r);
        }

        // Okay, a genuine update occured. We should inform gui objects so they can be updated if necessary.

        // (expand this for list when list exists) PLUS have ******race checking****.

        if (nNotifiables_ > 0)
        {
            // Looks like someone potentially cares that we've changed...........

            for (Notifiables::iterator i = notifiables_.begin(); i != notifiables_.end(); ++i)
            {
                // only notify an observer of the same race type as that for which the tree is being updated.
                if ((*i)->race() == r)
                {
                    (*i)->notifiableBeNotified();
                }
            }
        }
    }

    TEST_INVARIANT;
}

//////////////////////////////////////////////////////////////////////////////

void MachLogConstructionTree::addMe(MachLogNotifiable* pNewCTreeNot)
{
    CB_MachLogConstructionTree_DEPIMPL();

    notifiables_.push_back(pNewCTreeNot);
    ++nNotifiables_;

    TEST_INVARIANT;
}

//////////////////////////////////////////////////////////////////////////////

void MachLogConstructionTree::removeMe(MachLogNotifiable* pCurrentCTreeNot)
{
    CB_MachLogConstructionTree_DEPIMPL();

    Notifiables::iterator i = notifiables_.begin();
    bool found = false;
    while (! found && i != notifiables_.end())
    {
        if ((*i) == pCurrentCTreeNot)
        {
            notifiables_.erase(i);
            found = true;
            --nNotifiables_;
        }
        else
            ++i;
    }

    ASSERT(found, "MachLogConstructionTree::removeMe : couldn't find a match for pCurrentCTreeNot in notifiables_");

    TEST_INVARIANT;
}

//////////////////////////////////////////////////////////////////////////////

void MachLogConstructionTree::readAllItems(const SysPathName& treePath)
{
    CB_MachLogConstructionTree_DEPIMPL();

    SysMetaFile metaFile("mach1.met");

    std::unique_ptr<std::istream> pIstream;

    if (SysMetaFile::useMetaFile())
    {
        // pIstream = new SysMetaFileIstream( metaFile, treePath, ios::text );
        pIstream = std::unique_ptr<std::istream>(new SysMetaFileIstream(metaFile, treePath, std::ios::in));
    }
    else
    {
        ASSERT_FILE_EXISTS(treePath.c_str());
        // pIstream = new ifstream( treePath.c_str(), ios::text | ios::in );
        pIstream = std::unique_ptr<std::istream>(new std::ifstream(treePath.c_str(), std::ios::in));
    }

    UtlLineTokeniser parser(*pIstream, treePath);
    while (! parser.finished())
    {
        size_t lineSize = parser.tokens().size();
        MachLog::ObjectType consType;
        int subType = 0;
        size_t hwLevel = 0;
        MachPhys::WeaponCombo weaponCombo = MachPhys::N_WEAPON_COMBOS;

        if (lineSize > 0)
        {
            ASSERT_INFO(lineSize);
            ASSERT(
                (lineSize == 4 || lineSize == 3 || lineSize == 2),
                "Wrong number of tokens in construction tree file line (should be either 2 or 3).\n");

            consType = MachLogScenario::objectType(parser.tokens()[0]);
            if (lineSize >= 3)
            {
                subType = MachLogScenario::objectSubType(consType, parser.tokens()[1]);
                // hwLevel = _CONST_CAST(size_t, atoi( parser.tokens()[ 2 ].c_str() ) );
                hwLevel = atoi(parser.tokens()[2].c_str());

                /*
                if ( lineSize==4 )
                {
                    weaponCombo = MachLogScenario::weaponCombo( parser.tokens()[3] );
                }
                */
            }
            else
            {
                subType = 0;
                // hwLevel = _CONST_CAST(size_t, atoi( parser.tokens()[ 1 ].c_str() ) );
                hwLevel = atoi(parser.tokens()[1].c_str());
            }

            MachLogConstructionItem* pRI = new MachLogConstructionItem(consType, subType, hwLevel, weaponCombo);
            //          ASSERT_INFO( *pRI );
            constructionItems_.push_back(pRI);
        }
        parser.parseNextLine();
    }

    TEST_INVARIANT;
}

//////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachLogConstructionTree& t)
{

    o << "MachLogConstructionTree " << static_cast<const void*>(&t) << " start" << std::endl;
    o << " ConstructionItems.size() " << t.constructionItems().size() << std::endl;
    o << "MachLogConstructionTree " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

//////////////////////////////////////////////////////////////////////////////

void MachLogConstructionTree::CLASS_INVARIANT
{
    CB_MachLogConstructionTree_DEPIMPL();

    INVARIANT(this != nullptr);
    INVARIANT(nNotifiables_ == notifiables_.size());
}

//////////////////////////////////////////////////////////////////////////////

/* End CONSTREE.CPP **************************************************/
