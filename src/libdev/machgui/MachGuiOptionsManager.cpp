#include "MachGuiOptionsManager.hpp"

#include "machgui/gui.hpp"

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
}
