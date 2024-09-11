#include "machgui/IInputRegistry.hpp"

namespace MachGui
{

bool Shortcut::matches(const DevButtonEvent& be) const
{
    KeyWithModifiers keyWithMods(
        be.scanCode(),
        KeyModifierFlags::fromAltCtrlShiftState(be.wasAltPressed(), be.wasCtrlPressed(), be.wasShiftPressed()));
    return matches(keyWithMods);
}

class InputRegistry : public IInputRegistry
{
public:
    void load()
    {
        shortcuts_["ui-controlpanel-hide"] = {
            Device::KeyCode::LEFT_ARROW | Device::KeyModifier::Alt,
        };
        shortcuts_["ui-controlpanel-show"] = {
            Device::KeyCode::RIGHT_ARROW | Device::KeyModifier::Alt,
        };

        shortcuts_["commands-attack-trigger"] = {
            Device::KeyCode::KEY_A,
        };
        shortcuts_["commands-construct-trigger"] = {
            Device::KeyCode::KEY_C,
        };
        shortcuts_["commands-construct-rotate"] = {
            Device::KeyCode::SPACE,
            Device::KeyCode::KEY_R,
            Device::KeyCode::MOUSE_EXTRA1,
        };
        shortcuts_["commands-deconstruct-trigger"] = {
            Device::KeyCode::KEY_D,
        };
        shortcuts_["commands-deploy-trigger"] = {
            Device::KeyCode::KEY_E,
        };
        shortcuts_["commands-move-trigger"] = {
            Device::KeyCode::KEY_M,
        };
    }

    const Shortcut& getShortcut(ActionId id) final
    {
        const auto& data = shortcuts_;
        PRE(data.contains(id));
        return data.at(id);
    }

protected:
    std::unordered_map<std::string, Shortcut> shortcuts_;
};

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
