/*
 * E D I T C O N S T R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedConstructionEditor

    Construction mode edit.
*/

#ifndef _PLANETED_EDITCONSTR_HPP
#define _PLANETED_EDITCONSTR_HPP

#include "planeted/editactor.hpp"
#include "ctl/pvector.hpp"
#include "ctl/vector.hpp"
#include "device/butevent.hpp"
#include "mathex/degrees.hpp"
#include "planeted/scenfile.hpp"

class MachPhysConstruction;
class W4dComposite;
// class DevButtonEvent;
class SysPathName;
class ofstream;
class MexTransform3d;

class PedConstructionEditor : public PedActorEditor
// Canonical form revoked
{
public:
    PedConstructionEditor();
    ~PedConstructionEditor() override;

    void CLASS_INVARIANT;

    void processInput(const DevButtonEvent&) override;

    void displayKeyboardCtrls() override;
    // PRE( pSceneManager_ != NULL );
    // PRE( pPlanet_ != NULL );
    void displayModeInfo() override;

    void readScnFile(PedScenarioFile&) override;

    void writeScnFile(PedScenarioFile&) override;

protected:
    void initialiseActors();
    void processRotation(bool clockwise);
    W4dEntity* currentActor() override;
    void processCycle(PedActorEditor::CycleDir dir) override;
    void processSelection() override;
    void processDelete() override;
    void processRace() override;
    void createEntity(W4dEntity&, const MexTransform3d&) override;
    void rotateAfterMove() override;
    virtual void processHidden(bool hidden);
    void changeAllSolidities(W4dEntity::Solidity) override;

private:
    struct Construction
    {
        Construction(
            MachPhys::ConstructionType conType = MachPhys::HARDWARE_LAB,
            int subType = 0,
            size_t hardwareLevel = 1,
            string description = "",
            MachPhys::WeaponCombo weaponCombo = MachPhys::L_BOLTER)
            : constructionType_(conType)
            , subType_(subType)
            , hardwareLevel_(hardwareLevel)
            , description_(description)
            , weaponCombo_(weaponCombo)
        {
        }

        MachPhys::ConstructionType constructionType_;
        int subType_;
        size_t hardwareLevel_;
        string description_;
        MachPhys::WeaponCombo weaponCombo_;
    };

    // Mapping between physical construction, logical construction, and scenario file data for construction
    struct ConstructionMapping
    {
        ConstructionMapping() { }
        ConstructionMapping(MachPhysConstruction* con, Construction* data, PedScenarioFile::Construction ScnCon)
            : construction_(con)
            , data_(data)
            , scnConstruction_(ScnCon)
        {
        }
        MachPhysConstruction* construction_;
        Construction* data_;
        PedScenarioFile::Construction scnConstruction_;
    };

    using Constructions = ctl_pvector<Construction>;
    using ConstructionMappings = ctl_vector<ConstructionMapping>;

    friend ostream& operator<<(ostream& o, const PedConstructionEditor& t);

    PedConstructionEditor(const PedConstructionEditor&);
    PedConstructionEditor& operator=(const PedConstructionEditor&);

    MachPhysConstruction* createConstruction(W4dEntity&, const MexTransform3d&, const Construction&);
    void createMapping(MachPhysConstruction*, Construction*, PedScenarioFile::Construction);

    // Data
    Constructions constructionData_; // List of all legal construction types with valid sub types, hardware levels, etc.
    Constructions::iterator conDataIter_; // Iterator into list of all legal construction types
    ConstructionMappings
        constructionMap_; // List of actual constructions each with a mapping to their type and orientation
    ConstructionMappings::iterator mapIterator_; // Iterator to construction map
    MachPhysConstruction* pSelectedConstruction_; // Iterator index into list of actual constructions
};

#endif

/* End EDITCONSTR.HPP ***********************************************/
