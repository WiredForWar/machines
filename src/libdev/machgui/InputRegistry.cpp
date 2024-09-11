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
