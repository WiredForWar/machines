/*
 * E D I T A R T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedArtefactEditor

    Editor mode for artefacts
*/

#ifndef _EDITART_HPP
#define _EDITART_HPP

#include "planeted/editactor.hpp"
#include "ctl/pvector.hpp"
#include "ctl/vector.hpp"
#include "world4d/genrepos.hpp"
#include "machphys/machphys.hpp"
#include "mathex/degrees.hpp"

class MachPhysArtefact;
class MachPhysTerrainTile;
class W4dComposite;
class W4dEntity;
class MexTransform3d;
class ofstream;
class PedScenarioFile;

class PedArtefactEditor : public PedActorEditor
// Canonical form revoked
{
public:
    PedArtefactEditor();
    ~PedArtefactEditor() override;

    void CLASS_INVARIANT;

    void processInput(const DevButtonEvent&) override;
    // PRE( pSceneManager_ != NULL );
    // PRE( pPlanet_ != NULL );

    void displayModeInfo() override;
    // PRE( pSceneManager_ != NULL );
    // PRE( pPlanet_ != NULL );

    // virtual void preRenderUpdate();
    // virtual void displayModeInfo();

    void readScnFile(PedScenarioFile&) override;
    // PRE( cspFileName.existsAsFile() );

    void writeScnFile(PedScenarioFile&) override;

    void displayKeyboardCtrls() override;
    // virtual void changingMode();

    void createCeilingArtefact(const MachPhysTerrainTile& terrainTile);

    void deleteCeilingArtefact(const MachPhysTerrainTile& terrainTile);

protected:
    void initialiseActors();
    W4dEntity* currentActor() override;
    void processCycle(PedActorEditor::CycleDir dir) override;
    void processSelection() override;
    void processDelete() override;
    void processRace() override;
    virtual void processRotation(bool);
    void createEntity(W4dEntity&, const MexTransform3d&) override;
    void rotateAfterMove() override;
    void processHide(bool hidden) override;
    void changeAllSolidities(W4dEntity::Solidity) override;
    virtual void processHeightChange(int deltaz);

private:
    class ArtefactMapping
    {
    public:
        ArtefactMapping(W4dEntity* artefact = nullptr, MexDegrees orientation = 0)
            : artefact_(artefact)
            , orientation_(orientation)
        {
        }
        W4dEntity* artefact_;
        MexDegrees orientation_;
    };

    using ArtefactGroup = ctl_vector<ArtefactMapping>;
    using ArtefactTable = ctl_vector<ArtefactGroup>;

    struct artefactIndex
    {
        artefactIndex(ArtefactTable::iterator col, ArtefactGroup::iterator row)
            : col_(col)
            , row_(row)
        {
        }

        ArtefactGroup::iterator row_;
        ArtefactTable::iterator col_;
    };

    PedArtefactEditor(const PedArtefactEditor&);
    PedArtefactEditor& operator=(const PedArtefactEditor&);

    void createEntity(W4dEntity&, const MexTransform3d&, const string& key, MexDegrees orientation);

    //  void initialiseActors();
    void deleteArtefacts();
    //   MachPhys::ArtefactType artefactType( const string& type );
    int subType(const string& type);
    artefactIndex storeIndex(W4dEntity* pSelectedArtefact);

    // Reads artefact file to populate artefact repository
    void readArfFile(const SysPathName&) override;

    // Converts between a lanscape tile's name and the ceiling artefact associated with it
    // ( if there is one ) - else returns ""
    string tileToKey(const MachPhysTerrainTile& terrainTile);

    friend ostream& operator<<(ostream& o, const PedArtefactEditor& t);

    // Artefact artefactData_;

    W4dEntity* pSelectedArtefact_;
    W4dGenericRepository artefactStore_;
    ArtefactTable artefacts_;
    uint artefactIndex_;

    SysPathName arfFileName_;
};

#endif

/* End EDITART.HPP *************************************************/
