/*
 * T I L E D T O R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedTileEditor

    A brief description of the class should go in here
*/

#ifndef _PLANETED_TILEDTOR_HPP
#define _PLANETED_TILEDTOR_HPP

#include "base/base.hpp"

#include "planeted/editmode.hpp"

#include "system/pathname.hpp"
#include "device/butevent.hpp"
#include "mathex/line3d.hpp"
#include "ctl/list.hpp"
#include "ctl/pvector.hpp"

class PedArtefactEditor;

// Foward references
class W4dSceneManager;
class MexPoint2d;
class MachLogPlanet;
class MachPhysTerrainTile;
class PedTileMarker;

class PedTileEditor : public PedEditorMode
{
    // Canonical form revoked

public:
    PedTileEditor(PedArtefactEditor& artefactEditor);
    // POST( selectedTiles_.empty() );

    ~PedTileEditor() override;

    void CLASS_INVARIANT;

    void processInput(const DevButtonEvent&) override;
    // PRE( pSceneManager_ != NULL );
    // PRE( pPlanet_ != NULL );

    void displayKeyboardCtrls() override;

    void displayModeInfo() override;

    void preRenderUpdate() override;

    void readCspFile(const SysPathName&) override;
    // PRE( cspFileName.existsAsFile() );

    void writeCspFile(std::ofstream&) override;

    void changingMode() override;

private:
    enum CycleDir
    {
        NEXT,
        PREV,
        CURRENT
    };

    struct SelectedTile
    {
        SelectedTile()
            : xPos_(0)
            , yPos_(0)
            , pSelectedTile_(nullptr)
            , pSelIndicator_(nullptr)
        {
        }

        size_t xPos_;
        size_t yPos_;
        MachPhysTerrainTile* pSelectedTile_;
        PedTileMarker* pSelIndicator_;
    };

    void processAttatchCeilingArtefact(bool attatch);

    using SelectedTiles = ctl_list<SelectedTile*>;

    // Operation deliberately revoked
    PedTileEditor(const PedTileEditor&);
    PedTileEditor& operator=(const PedTileEditor&);
    bool operator==(const PedTileEditor&);

    void highlightTile();
    // PRE( pSceneManager_ != NULL );

    void processSelection(const DevButtonEvent& devButtonEvent);
    void processRotation(bool clockwise);
    void processHeightChange(bool up, bool shiftPressed);
    void processCycleTile(CycleDir dir);

    // Called by displayInfo
    void displayTileCoords();

    bool isSelected(MachPhysTerrainTile*);
    void clearAllSelectedTiles();

    friend ostream& operator<<(ostream& o, const PedTileEditor& t);

    // Data...
    PedTileMarker* pHighlightTile_;
    SelectedTiles selectedTiles_;
    PedArtefactEditor& artefactEditor_;
};

#endif

/* End TILEDTOR.HPP *************************************************/
