#include "MachGuiOptionsManager.hpp"

#include "afx/resource.hpp"
#include "gui/restring.hpp"
#include "machgui/gui.hpp"
#include "machlog/actor.hpp"
#include "machlog/races.hpp"
#include "machphys/marker.hpp"
#include "system/vfs.hpp"

#include "spdlog/spdlog.h"

#include <filesystem>
#include <system_error>

namespace
{

const std::string embeddedLang = "en";

} // namespace

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

void MachGuiOptionsManager::initializeLanguage()
{
    System::clearFsOverrides();

    AfxResourceLib& resource = GuiResourceString::resource();
    resource.clear();

    const std::string displayStringsFile = "machstrg.xml";
    resource.addStringsFromFile(displayStringsFile);

    std::string lang = SysRegistry::instance().queryStringValue("Options", "Language", embeddedLang);

    if ((lang != embeddedLang) && System::registerFsOverride("languages/" + lang))
    {
        std::string extraFile = System::findFile(displayStringsFile);
        if (extraFile != displayStringsFile)
        {
            resource.addStringsFromFile(extraFile);
        }
    }
}

std::vector<std::string> MachGuiOptionsManager::availableLanguages() const
{
    std::vector<std::string> result;
    result.push_back(embeddedLang);

    std::error_code code;
    std::filesystem::directory_iterator it(
        "languages",
        std::filesystem::directory_options::follow_directory_symlink,
        code);
    if (code)
    {
        spdlog::warn("Unable to list available languages (fs error: {})", code.message());
    }
    else
    {
        for (const std::filesystem::directory_entry& entry : it)
        {
            std::filesystem::path target = entry.is_symlink() ? std::filesystem::read_symlink(entry) : entry;
            if (!std::filesystem::is_directory(target))
                continue;

            result.push_back(entry.path());
        }
    }

    return result;
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
