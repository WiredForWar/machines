#pragma once

#include "IInputRegistry.hpp"

#include "machgui/InputLayout.hpp"

#include "gui/ResolvedUiString.hpp"

#include <unordered_map>
#include <vector>

namespace MachGui
{

class IBindsStorage;

class InputCategoryDetails
{
public:
    std::string displayName_;
    std::vector<BindId> bindIds_;
};

class BindDisplayData
{
public:
    ResolvedUiString displayName_;
    ResolvedUiString compactDisplayName_;
    std::string displayBind_;
};

class InputRegistry : public IInputRegistry
{
public:
    using KeyCode = Device::KeyCode;
    using KeyModifier = Device::KeyModifier;

    explicit InputRegistry(IBindsStorage* storage);

    void setLayout(InputLayout layout);

    void load();
    void save() const;

    const KeyBinds& getBinds(BindId id) const final;
    const std::string& getBindDisplayString(BindId id, DisplayFormat format = {}) const final;

    std::vector<std::string> getCategories() const;
    InputCategoryDetails getCategoryDetails(const std::string& name) const;

    void addCategory(std::string name, Gui::StringId stringId);
    void createBind(
        std::string_view category, BindId id, Gui::StringId stringId = {}, Gui::StringId compactStringId = {});
    void createSpecialBind(std::string_view category, BindId id, Gui::StringId stringId);
    void setBinds(BindId id, const KeyBinds& newBinds);

    BindDisplayData getBindDisplayData(BindId id, DisplayFormat format = {}) const;

protected:
    class BindData
    {
    public:
        Gui::StringId stringId_{};
        Gui::StringId compactStringId_{};
        KeyBinds binds_{};
        bool special_{};
    };

    class CategoryData
    {
    public:
        std::string name;
        Gui::StringId displayStringId_;
        std::vector<std::string> bindIds_{};
    };

    class string_hash
    {
    public:
        using is_transparent = void;
        [[nodiscard]] size_t operator()(std::string_view txt) const { return std::hash<std::string_view> {}(txt); }
        [[nodiscard]] size_t operator()(const std::string& txt) const { return std::hash<std::string> {}(txt); }
    };

protected:
    static std::string getKeysDisplayString(const KeyBinds& binds, DisplayFormat format);
    static std::string joinStringsWithOr(const std::string& str1, const std::string& str2);

    BindDisplayData getSpecialBindDisplayData(BindId id, const BindData& data, DisplayFormat format) const;
    std::string formatTwoKeys(BindId id1, BindId id2, DisplayFormat format) const;

    void initBinds();

    void setDefaults();

    void setLegacyDefaults();
    void setWasdLayout();

protected:
    std::unordered_map<std::string, BindData, string_hash, std::equal_to<>> binds_;
    mutable std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> bindDisplayStrings_;

    std::vector<CategoryData> categories_;
    IBindsStorage *storage_{};
    InputLayout layout_{};
    Utils::HandleWithTriggerUPtr layoutVarHandle_{};
};

InputRegistry* inputRegistryImpl();

} // namespace MachGui
