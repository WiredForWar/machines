#include "machgui/IInputRegistry.hpp"

#include "base/prepost.hpp"

#include <string>
#include <unordered_map>

namespace MachGui
{

class InputRegistry : public IInputRegistry
{
public:
    using KeyCode = Device::KeyCode;
    using KeyModifier = Device::KeyModifier;

    void load();

    const KeyBinds& getBinds(BindId id) const final;

protected:
    struct string_hash
    {
        using is_transparent = void;
        [[nodiscard]] size_t operator()(std::string_view txt) const { return std::hash<std::string_view> {}(txt); }
        [[nodiscard]] size_t operator()(const std::string& txt) const { return std::hash<std::string> {}(txt); }
    };

    std::unordered_map<std::string, KeyBinds, string_hash, std::equal_to<>> binds_;
};

void InputRegistry::load()
{
    binds_[std::string(NoBind)] = {};

    binds_["ui-controlpanel-hide"] = {
        { KeyCode::LEFT_ARROW | KeyModifier::Alt },
    };
    binds_["ui-controlpanel-show"] = {
        { KeyCode::RIGHT_ARROW | KeyModifier::Alt },
    };
    binds_["view-toggle-fpv"] = {
        { KeyCode::PAD_1 },
        { KeyCode::GRAVE },
    };

    binds_["commands-assembly-point-trigger"] = {
        { KeyCode::KEY_B },
    };
    binds_["commands-attack-trigger"] = {
        { KeyCode::KEY_A },
    };
    binds_["commands-build-trigger"] = {
        { KeyCode::KEY_B },
    };
    binds_["commands-camouflage-trigger"] = {
        { KeyCode::KEY_O },
    };
    binds_["commands-capture-trigger"] = {
        { KeyCode::KEY_U },
    };
    binds_["commands-construct-trigger"] = {
        { KeyCode::KEY_C },
    };
    binds_["commands-construct-rotate"] = {
        { KeyCode::SPACE },
        { KeyCode::KEY_R },
        { KeyCode::MOUSE_EXTRA1 },
    };
    binds_["commands-deconstruct-trigger"] = {
        { KeyCode::KEY_D },
    };
    binds_["commands-defcon-trigger"] = {
        { KeyCode::TAB },
    };
    binds_["commands-deploy-trigger"] = {
        { KeyCode::KEY_E },
    };
    binds_["commands-form-squadron-trigger"] = {
        { KeyCode::KEY_F },
    };
    binds_["commands-heal-trigger"] = {
        { KeyCode::KEY_H },
    };
    binds_["commands-ion-attack-trigger"] = {
        { KeyCode::KEY_I },
    };
    binds_["commands-drop-land-mine-trigger"] = {
        { KeyCode::KEY_M | KeyModifier::Shift },
    };
    binds_["commands-locate-trigger"] = {
        { KeyCode::KEY_L },
    };
    binds_["commands-move-trigger"] = {
        { KeyCode::KEY_M },
    };
    binds_["commands-nuke-attack-trigger"] = {
        { KeyCode::KEY_N },
    };
    binds_["commands-patrol-trigger"] = {
        { KeyCode::KEY_P },
    };
    binds_["commands-pick-up-trigger"] = {
        { KeyCode::KEY_K },
    };
    binds_["commands-recycle-trigger"] = {
        { KeyCode::KEY_Q },
    };
    binds_["commands-refill-land-mine-trigger"] = {
        { KeyCode::KEY_G },
    };
    binds_["commands-repair-trigger"] = {
        { KeyCode::KEY_R },
    };
    binds_["commands-scavenge-trigger"] = {
        { KeyCode::KEY_Y },
    };
    binds_["commands-self-destruct-trigger"] = {
        { KeyCode::BACK_SPACE | KeyModifier::Ctrl | KeyModifier::Shift },
    };
    binds_["commands-stand-ground-trigger"] = {
        { KeyCode::KEY_W },
    };
    binds_["commands-stop-trigger"] = {
        { KeyCode::KEY_S },
    };
    binds_["commands-transport-trigger"] = {
        { KeyCode::KEY_T },
    };
    binds_["commands-treachery-trigger"] = {
        { KeyCode::KEY_J },
    };
}

const KeyBinds& InputRegistry::getBinds(BindId id) const
{
    const auto& data = binds_;
    const std::string_view& asView = id;
    PRE_INFO(asView);
    const auto it = data.find(asView);
    PRE(it != data.end());
    return it->second;
}

IInputRegistry* inputRegistry()
{
    static InputRegistry r;
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
        r.load();
    }
    return &r;
}

} // namespace MachGui
