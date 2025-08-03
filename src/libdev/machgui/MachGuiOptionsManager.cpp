#include "MachGuiOptionsManager.hpp"

#include "machgui/gui.hpp"
#include "machlog/actor.hpp"
#include "machlog/races.hpp"
#include "machphys/marker.hpp"

MachGuiOptionsManager::MachGuiOptionsManager()
{
    SysRegistry::instance().addObserver(this);
}

MachGuiOptionsManager::~MachGuiOptionsManager()
{
    SysRegistry::instance().removeObserver(this);
}

void MachGuiOptionsManager::initializeCursor()
{
    bool use2DCursor = SysRegistry::instance().queryIntegerValue("Options\\Cursor Type", "2D");
    MachPhysMarker::setMarkerType(use2DCursor ? MachPhysMarker::MarkerType::TwoD : MachPhysMarker::MarkerType::ThreeD);

    float lineWidth = MachGui::getPhysMarkerLineWidth();
    MachPhysMarker::setMarkerLineWidth(lineWidth);
}

void MachGuiOptionsManager::onChanges()
{
    bool use2DCursor = SysRegistry::instance().queryIntegerValue("Options\\Cursor Type", "2D");
    using MarkerType = MachPhysMarker::MarkerType;
    const MarkerType wantedType = use2DCursor ? MarkerType::TwoD : MarkerType::ThreeD;
    if (MachPhysMarker::markerType() != wantedType)
    {
        initializeCursor();
        recreatePhysMarkers();
    }
}

void MachGuiOptionsManager::recreatePhysMarkers()
{
    MachLogRaces::Objects& allObjects = MachLogRaces::instance().objects();

    for (MachLogRaces::Objects::iterator iter = allObjects.begin(); iter != allObjects.end(); ++iter)
    {
        MachActor* pActor = *iter;
        if (pActor->selectionState() == MachLog::SELECTED)
        {
            // Deselect, then reselect to refresh bounding box
            pActor->selectionState(MachLog::NOT_SELECTED);
            pActor->selectionState(MachLog::SELECTED);
        }
        else if (pActor->selectionState() == MachLog::HIGHLIGHTED)
        {
            // Deselect, then reselect to refresh bounding box
            pActor->selectionState(MachLog::NOT_SELECTED);
            pActor->selectionState(MachLog::HIGHLIGHTED);
        }
    }
}
