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
