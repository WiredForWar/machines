#include "IInputRegistry.hpp"

#include "gui/StringId.hpp"

#include <unordered_map>
#include <vector>

namespace MachGui
{

class InputCategoryDetails
{
public:
    std::string displayName_;
    std::vector<BindId> bindIds_;
};

class BindDisplayData
{
public:
    std::string displayName_;
    std::string displayBind_;
};

class InputRegistry : public IInputRegistry
{
public:
    using KeyCode = Device::KeyCode;
    using KeyModifier = Device::KeyModifier;

    InputRegistry();

    const KeyBinds& getBinds(BindId id) const final;
    const std::string& getBindDisplayString(BindId id, DisplayFormat format = {}) const final;

    std::vector<std::string> getCategories() const;
    InputCategoryDetails getCategoryDetails(const std::string& name) const;

    void addCategory(std::string name, Gui::StringId stringId);
    void createBind(std::string_view category, BindId id, Gui::StringId stringId = {});
    void createSpecialBind(std::string_view category, BindId id, Gui::StringId stringId);
    void setBinds(BindId id, const KeyBinds& newBinds);

    BindDisplayData getBindDisplayData(BindId id, DisplayFormat format = {}) const;

protected:
    class BindData
    {
    public:
        Gui::StringId stringId_;
        KeyBinds binds_;
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

    BindDisplayData getSpecialBindDisplayData(BindId id, const BindData& data, DisplayFormat format) const;

    void initBinds();

    void setDefaults();

protected:
    std::unordered_map<std::string, BindData, string_hash, std::equal_to<>> binds_;
    mutable std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> bindDisplayStrings_;

    std::vector<CategoryData> categories_;
};

InputRegistry* inputRegistryImpl();

} // namespace MachGui
