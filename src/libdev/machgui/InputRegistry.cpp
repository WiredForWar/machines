#include "machgui/InputRegistry.hpp"

#include "machgui/BindsStorageXml.hpp"
#include "machgui/internal/strings.hpp"

#include "gui/ResolvedUiString.hpp"
#include "gui/gui.hpp"
#include "gui/restring.hpp"

#include "base/prepost.hpp"

#include "utility/string.hpp"

#include <algorithm>
#include <unordered_map>

namespace MachGui
{

namespace
{

constexpr char bcGeneral[] = "general";
constexpr char bcFirstPerson[] = "fpv";

bool isArrowKey(Device::KeyCode code)
{
    switch (code)
    {
        case Device::KeyCode::UP_ARROW:
        case Device::KeyCode::DOWN_ARROW:
        case Device::KeyCode::LEFT_ARROW:
        case Device::KeyCode::RIGHT_ARROW:
            return true;
        default:
            break;
    }

    return false;
}

} // namespace

InputRegistry::InputRegistry(IBindsStorage *storage)
    : storage_(storage)
{
    initBinds();

    layoutVarHandle_ = Config::inputBaseLayout.addListener([this]() {setLayout(Config::inputBaseLayout.get());});
    layoutVarHandle_->trigger();
}

void InputRegistry::setLayout(InputLayout layout)
{
    layout_ = layout;

    load();
}

void InputRegistry::load()
{
    setDefaults();

    bindDisplayStrings_.clear();

    for (auto& [bindId, bindData] : binds_)
    {
        if (bindData.special_)
            continue;

        storage_->read(BindId(bindId), &bindData.binds_);
    }
}

void InputRegistry::save() const
{
    for (const auto& [bindId, bindData] : binds_)
    {
        if (bindData.special_)
            continue;

        storage_->write(BindId(bindId), bindData.binds_);
    }

    storage_->sync();
}

const KeyBinds& InputRegistry::getBinds(BindId id) const
{
    const auto& data = binds_;
    const std::string_view& asView = id;
    PRE_INFO(asView);
    const auto it = data.find(asView);
    PRE(it != data.end());
    return it->second.binds_;
}

const std::string& InputRegistry::getBindDisplayString(BindId id, DisplayFormat format) const
{
    const auto it = bindDisplayStrings_.find(id);
    if (it != bindDisplayStrings_.end())
        return it->second;

    std::string idAsString = std::string(id);
    const KeyBinds& binds = getBinds(id);

    return bindDisplayStrings_[idAsString] = getKeysDisplayString(binds, format);
}

std::vector<std::string> InputRegistry::getCategories() const
{
    std::vector<std::string> result;
    result.reserve(categories_.size());
    for (const CategoryData& data : categories_)
        result.push_back(data.name);

    return result;
}

InputCategoryDetails InputRegistry::getCategoryDetails(const std::string& name) const
{
    const auto categoryIt = std::find_if(categories_.cbegin(), categories_.cend(), [name](const CategoryData& data) {
        return data.name == name;
    });
    PRE_INFO(name);
    PRE(!name.empty());
    PRE(categoryIt != categories_.cend());
    if (categoryIt == categories_.cend())
        return {};

    const CategoryData& data = *categoryIt;
    InputCategoryDetails details;
    details.displayName_ = GuiResourceString(data.displayStringId_).asString();
    details.bindIds_.reserve(data.bindIds_.size());
    for (const std::string& bind : data.bindIds_)
        details.bindIds_.push_back(BindId(bind));

    return details;
}

void InputRegistry::addCategory(std::string name, Gui::StringId stringId)
{
    const auto categoryIt = std::find_if(categories_.cbegin(), categories_.cend(), [name](const CategoryData& data) {
        return data.name == name;
    });
    PRE_INFO(name);
    PRE(!name.empty());
    PRE(categoryIt == categories_.cend());
    if (categoryIt != categories_.cend())
        return;

    categories_.emplace_back(
        CategoryData {
            .name = name,
            .displayStringId_ = stringId,
        });
}

void InputRegistry::createBind(
    std::string_view category, BindId id, Gui::StringId stringId, Gui::StringId compactStringId)
{
    std::string idAsString(id);
    BindData& data = binds_[idAsString];
    data.stringId_ = stringId;
    data.compactStringId_ = compactStringId;

    const auto categoryIt
        = std::find_if(categories_.begin(), categories_.end(), [category](const CategoryData& data) -> bool {
        return data.name == category;
    });

    PRE_INFO(category);
    PRE(categoryIt != categories_.end());
    if (categoryIt != categories_.end())
    {
        CategoryData& categoryDetails = *categoryIt;
        categoryDetails.bindIds_.push_back(idAsString);
    }
}

void InputRegistry::createSpecialBind(std::string_view category, BindId id, Gui::StringId stringId)
{
    createBind(category, id, stringId);

    const auto it = binds_.find(id);
    it->second.special_ = true;
}

void InputRegistry::setBinds(BindId id, const KeyBinds &newBinds)
{
    PRE_INFO(id);
    const auto it = binds_.find(id);
    PRE(it != binds_.end());

    if (it == binds_.end())
        return;

    it->second.binds_ = newBinds;
}

BindDisplayData InputRegistry::getBindDisplayData(BindId id, DisplayFormat format) const
{
    PRE_INFO(id);
    const auto it = binds_.find(id);
    PRE(it != binds_.end());

    if (it == binds_.end())
        return {};

    const BindData& data = it->second;
    if (data.stringId_ == 0)
        return {};

    if (data.special_)
        return getSpecialBindDisplayData(id, data, format);

    return {
        .displayName_ = data.stringId_,
        .compactDisplayName_ = data.compactStringId_,
        .displayBind_ = getKeysDisplayString(data.binds_, format),
    };
}

std::string InputRegistry::getKeysDisplayString(const KeyBinds &binds, DisplayFormat format)
{
    if (binds.empty())
        return {};

    if (binds.size() == 1)
        return toDisplayString(binds.at(0), format);
    else
        return joinStringsWithOr(toDisplayString(binds.at(0), format), toDisplayString(binds.at(1), format));
}

std::string InputRegistry::joinStringsWithOr(std::string str1, std::string str2)
{
    GuiStrings strings;
    strings.emplace_back(std::move(str1));
    strings.emplace_back(std::move(str2));

    return GuiResourceString(IDS_KEYS_OR, strings).asString();
}

BindDisplayData InputRegistry::getSpecialBindDisplayData(BindId id, const BindData& data, DisplayFormat format) const
{
    PRE_INFO(id);

    BindDisplayData result;
    if (id == "x-view-save"_bind)
    {
        result.displayName_ = GuiResourceString(data.stringId_, GuiStrings{ "1", "4" }).asString();
        result.displayBind_ = toDisplayString(KeyCode::F5) + "-" + toDisplayString(KeyCode::F8) + " "
            + toDisplayString(KeyModifier::Ctrl);
    }
    else if (id == "x-view-restore"_bind)
    {
        result.displayName_ = GuiResourceString(data.stringId_, GuiStrings{ "1", "4" }).asString();
        result.displayBind_ = toDisplayString(KeyCode::F5) + "-" + toDisplayString(KeyCode::F8);
    }
    else if (id == "x-view-next"_bind)
    {
        result.displayBind_ = formatTwoKeys("view-next-machine"_bind, "view-next-construction"_bind, format);
    }
    else if (id == "x-select-visible"_bind)
    {
        result.displayBind_ = formatTwoKeys("select-visible-machines"_bind, "select-visible-constructions"_bind, format);
        if (!result.displayBind_.empty())
        {
            result.displayBind_ += " (" + toDisplayString(KeyModifier::Ctrl) + ")";
        }
    }
    else if (id == "x-select-all"_bind)
    {
        result.displayBind_ = formatTwoKeys("select-all-machines"_bind, "select-all-constructions"_bind, format);
        if (!result.displayBind_.empty())
        {
            result.displayBind_ += " (" + toDisplayString(KeyModifier::Ctrl) + ")";
        }
    }
    else if (id == "x-deselect-all"_bind)
    {
        result.displayBind_ = toDisplayString(KeyCode::MOUSE_RIGHT, format);
    }
    else if (id == "x-squadron-create"_bind)
    {
        result.displayName_ = GuiResourceString(data.stringId_, GuiStrings{ "0", "9" }).asString();
        std::string combinedBind = toDisplayString(KeyCode::KEY_0) + "-" + toDisplayString(KeyCode::KEY_9)
            + " " + toDisplayString(KeyModifier::Ctrl);

        const auto command = getBinds("commands-form-squadron"_bind);
        if (command.empty())
        {
            result.displayBind_ = combinedBind;
        }
        else
        {
            result.displayBind_ = joinStringsWithOr(std::move(combinedBind), toDisplayString(command.at(0)));
        }
    }
    else if (id == "x-squadron-add"_bind)
    {
        result.displayName_ = GuiResourceString(data.stringId_, GuiStrings{ "0", "9" }).asString();
        result.displayBind_ = toDisplayString(KeyCode::KEY_0) + "-" + toDisplayString(KeyCode::KEY_9) + " "
            + toDisplayString(KeyModifier::Shift);
    }
    else if (id == "x-squadron-select"_bind)
    {
        result.displayName_ = GuiResourceString(data.stringId_, GuiStrings{ "0", "9" }).asString();
        result.displayBind_ = toDisplayString(KeyCode::KEY_0) + "-" + toDisplayString(KeyCode::KEY_9);
    }
    else if (id == "x-move-camera"_bind)
    {
        const auto& keyUp = getBinds("zenith-camera-forward"_bind);
        const auto& keyDown = getBinds("zenith-camera-backward"_bind);
        const auto& keyLeft = getBinds("zenith-camera-slide-left"_bind);
        const auto& keyRight = getBinds("zenith-camera-slide-right"_bind);

        std::vector<std::string> keyStrings;
        if (!keyUp.empty() && isArrowKey(keyUp.at(0).keyCode()))
        {
            // Order arrows as Up/Down/Left/Right
            keyStrings = {
                keyUp.empty() ? std::string() : toDisplayString(keyUp.at(0), format),
                keyDown.empty() ? std::string() : toDisplayString(keyDown.at(0), format),
                keyLeft.empty() ? std::string() : toDisplayString(keyLeft.at(0), format),
                keyRight.empty() ? std::string() : toDisplayString(keyRight.at(0), format),
            };
        }
        else
        {
            // Order other keys as Up/Left/Down/Right (e.g. WASD or ESDF)
            keyStrings = {
                keyUp.empty() ? std::string() : toDisplayString(keyUp.at(0), format),
                keyLeft.empty() ? std::string() : toDisplayString(keyLeft.at(0), format),
                keyDown.empty() ? std::string() : toDisplayString(keyDown.at(0), format),
                keyRight.empty() ? std::string() : toDisplayString(keyRight.at(0), format),
            };
        }

        result.displayBind_ = Utils::join(keyStrings, " ") + " " + "(" + toDisplayString(KeyModifier::Shift) + ")";
    }
    else if (id == "x-map-move-camera"_bind)
    {
        result.displayBind_ = toDisplayString(KeyCode::MOUSE_RIGHT);
    }
    else if (id == "x-ui-controlpanel-toggle"_bind)
    {
        result.displayBind_ = formatTwoKeys("ui-controlpanel-hide"_bind, "ui-controlpanel-show"_bind, format);
    }
    else if (id == "x-send-chat-to-target"_bind)
    {
        result.displayBind_ = Utils::join<std::vector<std::string>>(
            {
                toDisplayString(KeyCode::F1),
                toDisplayString(KeyCode::F2),
                toDisplayString(KeyCode::F3),
                toDisplayString(KeyCode::F4),
            },
            "/");
    }
    else if(id == "x-alliance-toggle"_bind)
    {
        result.displayBind_ = Utils::join<std::vector<std::string>>(
            {
                toDisplayString(KeyCode::F1),
                toDisplayString(KeyCode::F2),
                toDisplayString(KeyCode::F3),
            },
            "/") + " " + toDisplayString(KeyModifier::Ctrl);
    }
    else if (id == "x-fpv-slow"_bind)
    {
        result.displayBind_ = toDisplayString(KeyModifier::Ctrl);
    }

    if (result.displayName_.empty())
        result.displayName_ = data.stringId_;

    POST(!result.displayName_.empty())

    return result;
}

std::string InputRegistry::formatTwoKeys(BindId id1, BindId id2, DisplayFormat format) const
{
    const auto& bind1 = getBinds(id1);
    const auto& bind2 = getBinds(id2);
    if (bind1.empty() || bind2.empty())
        return {};

    const auto keyWithMods1 = bind1.at(0).keyWithMods;
    const auto keyWithMods2 = bind2.at(0).keyWithMods;

    if (keyWithMods1 == keyWithMods2)
        return toDisplayString(bind1.at(0), format);

    const std::string delimiter = isArrowKey(keyWithMods1.keyCode()) ? " " : "/";
    if (keyWithMods1.modifiers() == keyWithMods2.modifiers())
    {
        std::string result;
        result = toDisplayString(bind1.at(0).keyCode(), format) + delimiter
            + toDisplayString(bind2.at(0).keyCode(), format);

        for (const KeyModifier mod : KeyModifierFlags::Order)
        {
            if (keyWithMods1.modifiers() & mod)
                result += " " + toDisplayString(mod, format);
        }

        return result;
    }
    else
    {
        return toDisplayString(bind1.at(0), format) + delimiter + toDisplayString(bind2.at(0), format);
    }
}

void InputRegistry::initBinds()
{
    binds_[std::string(NoBind)] = {};

    addCategory(bcGeneral, IDS_MENU_GENERALCONTROL);
    addCategory(bcFirstPerson, IDS_MENU_FIRSTPERSONCONTROL);

    createBind(bcGeneral, "view-use-zenith-camera"_bind, IDS_ZENITH_VIEW);
    createBind(bcGeneral, "view-toggle-fpv"_bind, IDS_FIRST_PERSON_VIEW);
    createBind(bcGeneral, "view-use-ground-camera"_bind, IDS_GROUND_VIEW);

    for (int i = 0; i < 4; ++i)
    {
        const std::string indexStr = std::to_string(i + 1);
        createBind(bcGeneral, BindId("view-save-view-" + indexStr));
        createBind(bcGeneral, BindId("view-restore-view-" + indexStr));
    }

    createSpecialBind(bcGeneral, "x-view-save"_bind, IDS_SAVE_VIEW_FROM_X_TO_Y);
    createSpecialBind(bcGeneral, "x-view-restore"_bind, IDS_RESTORE_VIEW_FROM_X_TO_Y);

    createSpecialBind(bcGeneral, "x-view-next"_bind, IDS_VIEW_NEXT);
    createBind(bcGeneral, "view-next-machine"_bind);
    createBind(bcGeneral, "view-next-construction"_bind);

    createBind(bcGeneral, "select-all-machines"_bind);
    createBind(bcGeneral, "select-all-constructions"_bind);
    createBind(bcGeneral, "add-all-machines"_bind);
    createBind(bcGeneral, "add-all-constructions"_bind);

    createBind(bcGeneral, "select-visible-machines"_bind);
    createBind(bcGeneral, "select-visible-constructions"_bind);
    createBind(bcGeneral, "add-visible-machines"_bind);
    createBind(bcGeneral, "add-visible-constructions"_bind);

    createSpecialBind(bcGeneral, "x-select-visible"_bind, IDS_SELECT_VISIBLE);
    createSpecialBind(bcGeneral, "x-select-all"_bind, IDS_SELECT_ALL);
    createSpecialBind(bcGeneral, "x-deselect-all"_bind, IDS_DESELECT_ALL);
    createBind(bcGeneral, "alternative-cursor"_bind, IDS_ALT_CURSOR);
    createSpecialBind(bcGeneral, "x-squadron-create"_bind, IDS_SQUADRON_CREATE_X_TO_Y);
    createSpecialBind(bcGeneral, "x-squadron-add"_bind, IDS_SQUADRON_ADD_X_TO_Y);
    createSpecialBind(bcGeneral, "x-squadron-select"_bind, IDS_SQUADRON_SELECT_X_TO_Y);

    createBind(bcGeneral, "commands-form-squadron"_bind);
    for (int i = 0; i <= 9; ++i)
    {
        const std::string bindId = std::to_string(i);
        createBind(bcGeneral, BindId("squadron-create-" + bindId));
        createBind(bcGeneral, BindId("squadron-select-" + bindId));
        createBind(bcGeneral, BindId("squadron-add-units-" + bindId));
    }

    createSpecialBind(bcGeneral, "x-move-camera"_bind, IDS_MOVE_CAMERA);
    createSpecialBind(bcGeneral, "x-map-move-camera"_bind, IDS_MAP_MOVE_CAMERA);

    createBind(bcGeneral, "view-restore-default-pos"_bind, IDS_VIEW_HOME);
    createBind(bcGeneral, "view-restore-last-pos"_bind);

    createBind(bcGeneral, "zenith-camera-forward"_bind);
    createBind(bcGeneral, "zenith-camera-backward"_bind);
    createBind(bcGeneral, "zenith-camera-slide-left"_bind);
    createBind(bcGeneral, "zenith-camera-slide-right"_bind);
    createBind(bcGeneral, "zenith-camera-rotate-left"_bind);
    createBind(bcGeneral, "zenith-camera-rotate-right"_bind);
    createBind(bcGeneral, "zenith-camera-up"_bind);
    createBind(bcGeneral, "zenith-camera-down"_bind);

    createBind(bcGeneral, "ground-camera-forward"_bind);
    createBind(bcGeneral, "ground-camera-backward"_bind);
    createBind(bcGeneral, "ground-camera-slide-left"_bind);
    createBind(bcGeneral, "ground-camera-slide-right"_bind);
    createBind(bcGeneral, "ground-camera-rotate-left"_bind);
    createBind(bcGeneral, "ground-camera-rotate-right"_bind);
    createBind(bcGeneral, "ground-camera-up"_bind);
    createBind(bcGeneral, "ground-camera-down"_bind);
    createBind(bcGeneral, "ground-camera-pitch-up"_bind);
    createBind(bcGeneral, "ground-camera-pitch-down"_bind);

    createBind(bcGeneral, "free-camera-forward"_bind);
    createBind(bcGeneral, "free-camera-backward"_bind);
    createBind(bcGeneral, "free-camera-slide-left"_bind);
    createBind(bcGeneral, "free-camera-slide-right"_bind);
    createBind(bcGeneral, "free-camera-rotate-left"_bind);
    createBind(bcGeneral, "free-camera-rotate-right"_bind);

    createBind(bcGeneral, "free-camera-up"_bind);
    createBind(bcGeneral, "free-camera-down"_bind);
    createBind(bcGeneral, "free-camera-pitch-up"_bind);
    createBind(bcGeneral, "free-camera-pitch-down"_bind);
    createBind(bcGeneral, "free-camera-roll-left"_bind);
    createBind(bcGeneral, "free-camera-roll-right"_bind);
    createBind(bcGeneral, "free-camera-reset-position"_bind);
    createBind(bcGeneral, "free-camera-reset-orientation"_bind);
    createBind(bcGeneral, "free-camera-stop-moving"_bind);
    createBind(bcGeneral, "free-camera-stop-rotating"_bind);

    createBind(bcGeneral, "ui-controlpanel-hide"_bind);
    createBind(bcGeneral, "ui-controlpanel-show"_bind);

    createBind(bcGeneral, "show-menus"_bind, IDS_MENUS);
    createSpecialBind(bcGeneral, "x-ui-controlpanel-toggle"_bind, IDS_TOGGLE_PANEL);

    createBind(bcGeneral, "screenshot"_bind, IDS_MAKE_SCREENSHOT);
    createBind(bcGeneral, "chat-send-to-system"_bind);
    createSpecialBind(bcGeneral, "x-send-chat-to-target"_bind, IDS_SEND_CHAT_TO_TARGET);
    createSpecialBind(bcGeneral, "x-alliance-toggle"_bind, IDS_ALLIANCE_TOGGLE);

    createBind(bcGeneral, "commands-build"_bind);

    createBind(bcGeneral, "commands-move"_bind, IDS_MOVE_COMMAND, IDS_COMMAND_MOVE);
    createBind(bcGeneral, "commands-attack"_bind, IDS_ATTACK_COMMAND, IDS_COMMAND_ATTACK);
    createBind(bcGeneral, "commands-construct"_bind, IDS_CONSTRUCT_COMMAND, IDS_COMMAND_CONSTRUCT);
    createBind(bcGeneral, "commands-construct-rotate"_bind);
    createBind(bcGeneral, "commands-locate"_bind, IDS_LOCATETO_COMMAND, IDS_COMMAND_LOCATE);
    createBind(bcGeneral, "commands-patrol"_bind, IDS_PATROL_COMMAND, IDS_COMMAND_PATROL);
    createBind(bcGeneral, "commands-deploy"_bind, IDS_DEPLOY_COMMAND, IDS_COMMAND_DEPLOY);
    createBind(bcGeneral, "commands-pick-up"_bind, IDS_PICKUP_COMMAND, IDS_COMMAND_PICKUP);
    createBind(bcGeneral, "commands-transport"_bind, IDS_TRANSPORT_COMMAND, IDS_COMMAND_TRANSPORT);

    createBind(bcGeneral, "commands-self-destruct"_bind, IDS_SELF_DESTRUCT_COMMAND, IDS_COMMAND_SELF_DESTRUCT);
    createBind(bcGeneral, "commands-drop-land-mine"_bind, IDS_DROPLANDMINE_COMMAND, IDS_COMMAND_DROP_MINE);
    createBind(bcGeneral, "commands-refill-land-mine"_bind, IDS_REFILLLANDMINE_COMMAND, IDS_COMMAND_REFILL);
    createBind(bcGeneral, "commands-heal"_bind, IDS_HEAL_COMMAND, IDS_COMMAND_HEAL);
    createBind(bcGeneral, "commands-deconstruct"_bind, IDS_DECONSTRUCT_COMMAND, IDS_COMMAND_DECONSTRUCT);
    createBind(bcGeneral, "commands-recycle"_bind, IDS_RECYCLE_COMMAND, IDS_COMMAND_RECYCLE);
    createBind(bcGeneral, "commands-stop"_bind, IDS_STOP_COMMAND, IDS_COMMAND_STOP);
    createBind(bcGeneral, "commands-stand-ground"_bind, IDS_STANDGROUND_COMMAND, IDS_COMMAND_STAND_GROUND);
    createBind(bcGeneral, "commands-repair"_bind, IDS_REPAIR_COMMAND, IDS_COMMAND_REPAIR);
    createBind(bcGeneral, "commands-capture"_bind, IDS_CAPTURE_COMMAND, IDS_COMMAND_CAPTURE);
    createBind(bcGeneral, "commands-defcon"_bind, IDS_MACHINE_INITIATIVE_COMMAND_NAME, IDS_COMMAND_INITIATIVE_LEVEL);
    createBind(bcGeneral, "commands-ion-attack"_bind, IDS_ION_ATTACK_COMMAND, IDS_COMMAND_ION_ATTACK);
    createBind(bcGeneral, "commands-nuke-attack"_bind, IDS_NUKE_ATTACK_COMMAND, IDS_COMMAND_NUKE_ATTACK);
    createBind(bcGeneral, "commands-assembly-point"_bind, IDS_ASSEMBLEAT_COMMAND, IDS_COMMAND_ASSEMBLE);
    createBind(bcGeneral, "commands-scavenge"_bind, IDS_SCAVENGE_COMMAND, IDS_COMMAND_SCAVENGE);
    createBind(bcGeneral, "commands-camouflage"_bind, IDS_CAMOUFLAGE_COMMAND, IDS_COMMAND_CAMOUFLAGE);
    createBind(bcGeneral, "commands-treachery"_bind, IDS_TREACHERY_COMMAND, IDS_COMMAND_TREACHERY);

    createBind(bcFirstPerson, "fpv-fire"_bind, IDS_ACTION_FIRE);
    createBind(bcFirstPerson, "fpv-switch-weapon"_bind, IDS_ACTION_WEAPON_SWITCH);

    createBind(bcFirstPerson, "fpv-move-forward"_bind, IDS_FORWARD);
    createBind(bcFirstPerson, "fpv-move-backward"_bind, IDS_BACKWARD);

    createBind(bcFirstPerson, "fpv-turn-left"_bind);
    createBind(bcFirstPerson, "fpv-turn-right"_bind);

    createBind(bcFirstPerson, "fpv-turn-left-fast"_bind, IDS_TURN_LEFT);
    createBind(bcFirstPerson, "fpv-turn-right-fast"_bind, IDS_TURN_RIGHT);

    createBind(bcFirstPerson, "fpv-look-down-fast"_bind, IDS_LOOK_DOWN);
    createBind(bcFirstPerson, "fpv-look-up-fast"_bind, IDS_LOOK_UP);
    createBind(bcFirstPerson, "fpv-look-down"_bind);
    createBind(bcFirstPerson, "fpv-look-up"_bind);

    createBind(bcFirstPerson, "fpv-turn-head-left"_bind);
    createBind(bcFirstPerson, "fpv-turn-head-right"_bind);
    createBind(bcFirstPerson, "fpv-turn-head-left-fast"_bind);
    createBind(bcFirstPerson, "fpv-turn-head-right-fast"_bind);
    createBind(bcFirstPerson, "fpv-center-head"_bind, IDS_HEAD_CENTER);

    createSpecialBind(bcFirstPerson, "x-fpv-slow"_bind, IDS_FPV_SLOW);
    createBind(bcFirstPerson, "fpv-toggle-night-vision"_bind, IDS_ACTION_TOGGLE_NVG);

    createBind(bcFirstPerson, "fpv-command-select-next"_bind);
    createBind(bcFirstPerson, "fpv-command-select-previous"_bind);
    createBind(bcFirstPerson, "fpv-command-attack"_bind);
    createBind(bcFirstPerson, "fpv-command-move"_bind);
    createBind(bcFirstPerson, "fpv-command-follow"_bind);

    createBind(bcFirstPerson, "fpv-exit"_bind, IDS_EXIT);
    createBind(bcFirstPerson, "fpv-menus"_bind, IDS_MENUS);

    createBind(bcGeneral, "gfx-toggle-rendering"_bind);
    createBind(bcGeneral, "ui-toggle-console"_bind);
}

void InputRegistry::setDefaults()
{
    bindDisplayStrings_.clear();

    setLegacyDefaults();

    if (layout_ == InputLayout::WASD)
    {
        setWasdLayout();
    }
}

void InputRegistry::setLegacyDefaults()
{
    setBinds("ui-toggle-console"_bind, {{ KeyCode::F1 }});
    setBinds("ui-controlpanel-hide"_bind, {
        { KeyCode::LEFT_ARROW | KeyModifier::Alt },
    });
    setBinds("ui-controlpanel-show"_bind, {
        { KeyCode::RIGHT_ARROW | KeyModifier::Alt },
    });
    setBinds("show-menus"_bind, {
        { KeyCode::ESCAPE },
        { .keyWithMods = KeyCode::F10, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift | KeyModifier::Alt },
    });
    setBinds("view-toggle-fpv"_bind, {
        { KeyCode::PAD_1 },
        { KeyCode::GRAVE },
    });
    setBinds("screenshot"_bind, { { KeyCode::F12 | KeyModifier::Ctrl | KeyModifier::Shift } });
    setBinds("chat-send-to-system"_bind, { { KeyCode::F11 | KeyModifier::Ctrl | KeyModifier::Shift | KeyModifier::Alt } });

    setBinds("view-use-zenith-camera"_bind, { { KeyCode::PAD_2 } });
    setBinds("view-use-ground-camera"_bind, { { KeyCode::PAD_0 } });

    for (int i = 0; i < 4; ++i)
    {
        KeyCode keyCode = KeyCode::F5 + Device::KeyCodeOffset { i };
        const std::string indexStr = std::to_string(i + 1);
        setBinds(BindId("view-save-view-" + indexStr), {
            {
                .keyWithMods = keyCode | KeyModifier::Ctrl,
                .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift,
            },
        });
        setBinds(BindId("view-restore-view-" + indexStr), {
            {
                .keyWithMods = keyCode,
                .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift,
            },
        });
    }
    setBinds("view-restore-default-pos"_bind, {
        {
            .keyWithMods = KeyCode::KEY_H | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt,
        },
    });
    setBinds("view-restore-last-pos"_bind, {
        {
            .keyWithMods = KeyCode::KEY_H | KeyModifier::Ctrl | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Alt,
        },
    });

    setBinds("view-next-machine"_bind, {
        {
            .keyWithMods = KeyCode::KEY_A | KeyModifier::Alt,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });
    setBinds("view-next-construction"_bind, {
        {
            .keyWithMods = KeyCode::KEY_S | KeyModifier::Alt,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });
    setBinds("select-all-machines"_bind, {
        {
            .keyWithMods = KeyCode::KEY_A | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt,
        },
    });
    setBinds("select-all-constructions"_bind, {
        {
            .keyWithMods = KeyCode::KEY_S | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt,
        },
    });
    setBinds("add-all-machines"_bind, {
        {
            .keyWithMods = KeyCode::KEY_A | KeyModifier::Ctrl | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Alt,
        },
    });
    setBinds("add-all-constructions"_bind, {
        {
            .keyWithMods = KeyCode::KEY_S | KeyModifier::Ctrl | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Alt,
        },
    });
    setBinds("select-visible-machines"_bind, {
        {
            .keyWithMods = KeyCode::KEY_Z | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt,
        },
    });
    setBinds("select-visible-constructions"_bind, {
        {
            .keyWithMods = KeyCode::KEY_X | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt,
        },
    });
    setBinds("add-visible-machines"_bind, {
        {
            .keyWithMods = KeyCode::KEY_Z | KeyModifier::Ctrl | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Alt,
        },
    });
    setBinds("add-visible-constructions"_bind, {
        {
            .keyWithMods = KeyCode::KEY_X | KeyModifier::Ctrl | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Alt,
        },
    });

    setBinds("alternative-cursor"_bind, { KeyBind { .keyWithMods = KeyWithModifiers({}, KeyModifier::Alt) } });

    setBinds("commands-assembly-point"_bind, {
        { KeyCode::KEY_B },
    });
    setBinds("commands-attack"_bind, {
        { KeyCode::KEY_A },
    });
    setBinds("commands-build"_bind, {
        { KeyCode::KEY_B },
    });
    setBinds("commands-camouflage"_bind, {
        { KeyCode::KEY_O },
    });
    setBinds("commands-capture"_bind, {
        { KeyCode::KEY_U },
    });
    setBinds("commands-construct"_bind, {
        { KeyCode::KEY_C },
    });
    setBinds("commands-construct-rotate"_bind, {
        { KeyCode::SPACE },
        { KeyCode::KEY_R },
        { KeyCode::MOUSE_EXTRA1 },
    });
    setBinds("commands-deconstruct"_bind, {
        { KeyCode::KEY_D },
    });
    setBinds("commands-defcon"_bind, {
        { KeyCode::TAB },
    });
    setBinds("commands-deploy"_bind, {
        { KeyCode::KEY_E },
    });
    setBinds("commands-form-squadron"_bind, {
        { KeyCode::KEY_F },
    });
    setBinds("commands-heal"_bind, {
        { KeyCode::KEY_H },
    });
    setBinds("commands-ion-attack"_bind, {
        { KeyCode::KEY_I },
    });
    setBinds("commands-drop-land-mine"_bind, {
        { KeyCode::KEY_M | KeyModifier::Shift },
    });
    setBinds("commands-locate"_bind, {
        { KeyCode::KEY_L },
    });
    setBinds("commands-move"_bind, {
        { KeyCode::KEY_M },
    });
    setBinds("commands-nuke-attack"_bind, {
        { KeyCode::KEY_N },
    });
    setBinds("commands-patrol"_bind, {
        { KeyCode::KEY_P },
    });
    setBinds("commands-pick-up"_bind, {
        { KeyCode::KEY_K },
    });
    setBinds("commands-recycle"_bind, {
        { KeyCode::KEY_Q },
    });
    setBinds("commands-refill-land-mine"_bind, {
        { KeyCode::KEY_G },
    });
    setBinds("commands-repair"_bind, {
        { KeyCode::KEY_R },
    });
    setBinds("commands-scavenge"_bind, {
        { KeyCode::KEY_Y },
    });
    setBinds("commands-self-destruct"_bind, {
        { KeyCode::BACK_SPACE | KeyModifier::Ctrl | KeyModifier::Shift },
    });
    setBinds("commands-stand-ground"_bind, {
        { KeyCode::KEY_W },
    });
    setBinds("commands-stop"_bind, {
        { KeyCode::KEY_S },
    });
    setBinds("commands-transport"_bind, {
        { KeyCode::KEY_T },
    });
    setBinds("commands-treachery"_bind, {
        { KeyCode::KEY_J },
    });

    for (int i = 0; i <= 9; ++i)
    {
        Device::KeyCode keyCode = KeyCode::KEY_0 + Device::KeyCodeOffset { i };
        static_assert(KeyCode::KEY_9 == KeyCode::KEY_0 + Device::KeyCodeOffset { 9 });

        const std::string bindId = std::to_string(i);
        setBinds(BindId("squadron-create-" + bindId), {
            {
                .keyWithMods = keyCode | KeyModifier::Ctrl,
                .releasedModifiers = KeyModifier::Shift,
            },
        });
        setBinds(BindId("squadron-select-" + bindId), {
            {
                .keyWithMods = keyCode,
                .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
            },
        });
        setBinds(BindId("squadron-add-units-" + bindId), {
            {
                .keyWithMods = keyCode | KeyModifier::Shift,
                .releasedModifiers = KeyModifier::Ctrl,
            },
        });
    }

    setBinds("zenith-camera-forward"_bind, {
        { .keyWithMods = KeyCode::UP_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-backward"_bind, {
        { .keyWithMods = KeyCode::DOWN_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-slide-left"_bind, {
        { .keyWithMods = KeyCode::LEFT_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-slide-right"_bind, {
        { .keyWithMods = KeyCode::RIGHT_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-rotate-left"_bind, {
        { .keyWithMods = KeyCode::LEFT_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("zenith-camera-rotate-right"_bind, {
        { .keyWithMods = KeyCode::RIGHT_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("zenith-camera-up"_bind, {
        { .keyWithMods = KeyCode::PAGE_UP, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::PLUS_PAD, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-down"_bind, {
        { .keyWithMods = KeyCode::PAGE_DOWN, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::MINUS_PAD, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });

    setBinds("ground-camera-forward"_bind, {
        { .keyWithMods = KeyCode::UP_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("ground-camera-backward"_bind, {
        { .keyWithMods = KeyCode::DOWN_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("ground-camera-slide-left"_bind, {
        { .keyWithMods = KeyCode::DELETE, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-slide-right"_bind, {
        { .keyWithMods = KeyCode::END, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-rotate-left"_bind, {
        { .keyWithMods = KeyCode::LEFT_ARROW, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-rotate-right"_bind, {
        { .keyWithMods = KeyCode::RIGHT_ARROW, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-up"_bind, {
        { .keyWithMods = KeyCode::PAGE_UP, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::PLUS_PAD, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-down"_bind, {
        { .keyWithMods = KeyCode::PAGE_DOWN, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::MINUS_PAD, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-pitch-up"_bind, {
        { .keyWithMods = KeyCode::UP_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-pitch-down"_bind, {
        { .keyWithMods = KeyCode::DOWN_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
    });

    setBinds("free-camera-forward"_bind, { { KeyCode::ENTER_PAD } });
    setBinds("free-camera-backward"_bind, { { KeyCode::DELETE } });
    setBinds("free-camera-slide-left"_bind, { { KeyCode::END } });
    setBinds("free-camera-slide-right"_bind, { { KeyCode::PAGE_DOWN } });
    setBinds("free-camera-rotate-left"_bind, { { KeyCode::LEFT_ARROW } });
    setBinds("free-camera-rotate-right"_bind, { { KeyCode::RIGHT_ARROW } });

    setBinds("free-camera-up"_bind, { { KeyCode::PLUS_PAD } });
    setBinds("free-camera-down"_bind, { { KeyCode::MINUS_PAD } });
    setBinds("free-camera-pitch-up"_bind, { { KeyCode::DOWN_ARROW } });
    setBinds("free-camera-pitch-down"_bind, { { KeyCode::UP_ARROW } });
    setBinds("free-camera-roll-left"_bind, { { KeyCode::HOME } });
    setBinds("free-camera-roll-right"_bind, { { KeyCode::PAGE_UP } });
    setBinds("free-camera-reset-position"_bind, { { KeyCode::KEY_R | KeyModifier::Shift } });
    setBinds("free-camera-reset-orientation"_bind, {
        { .keyWithMods = KeyCode::KEY_R, .releasedModifiers = KeyModifier::Shift },
    });
    setBinds("free-camera-stop-moving"_bind, { { KeyCode::INSERT } });
    setBinds("free-camera-stop-rotating"_bind, { { KeyCode::PAD_5 } });

    setBinds("fpv-fire"_bind, { { KeyCode::SPACE } });
    setBinds("fpv-center-head"_bind, { { KeyCode::PAD_5 } });
    setBinds("fpv-switch-weapon"_bind, { { KeyCode::TAB } });

    setBinds("fpv-move-forward"_bind, {
        { .keyWithMods = KeyCode::UP_ARROW, .releasedModifiers = KeyModifier::Shift },
        { .keyWithMods = KeyCode::UP_ARROW_PAD, .releasedModifiers = KeyModifier::Shift },
    });

    setBinds("fpv-move-backward"_bind, {
        { .keyWithMods = KeyCode::DOWN_ARROW, .releasedModifiers = KeyModifier::Shift },

        // Removed by NA 3/2/99 so that switch to zenith works. obviously you can no longer reverse the machine
        // using the pad down arrow key.
        // { .keyWithMods = KeyCode::DOWN_ARROW_PAD, .releasedModifiers = KeyModifier::Shift },
    });

    setBinds("fpv-look-down-fast"_bind, {
        { .keyWithMods = KeyCode::UP_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl },
    });
    setBinds("fpv-look-up-fast"_bind, {
        { .keyWithMods = KeyCode::DOWN_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl },
    });
    setBinds("fpv-look-down"_bind, {
        { KeyCode::UP_ARROW | KeyModifier::Ctrl | KeyModifier::Shift },
    });
    setBinds("fpv-look-up"_bind, {
        { KeyCode::DOWN_ARROW | KeyModifier::Ctrl | KeyModifier::Shift },
    });

    setBinds("fpv-turn-left"_bind, {
        {
            .keyWithMods = KeyCode::LEFT_ARROW | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::LEFT_ARROW_PAD | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
    });
    setBinds("fpv-turn-right"_bind, {
        {
            .keyWithMods = KeyCode::RIGHT_ARROW | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::RIGHT_ARROW_PAD | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
    });

    setBinds("fpv-turn-left-fast"_bind, {
        {
            .keyWithMods = KeyCode::LEFT_ARROW,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::LEFT_ARROW_PAD,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });
    setBinds("fpv-turn-right-fast"_bind, {
        {
            .keyWithMods = KeyCode::RIGHT_ARROW,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::RIGHT_ARROW_PAD,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });

    setBinds("fpv-turn-head-left"_bind, {
        {
            KeyCode::LEFT_ARROW | KeyModifier::Ctrl | KeyModifier::Shift,
        },
        {
            KeyCode::LEFT_ARROW_PAD | KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });
    setBinds("fpv-turn-head-right"_bind, {
        {
            KeyCode::RIGHT_ARROW | KeyModifier::Ctrl | KeyModifier::Shift,
        },
        {
            KeyCode::RIGHT_ARROW_PAD | KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });
    setBinds("fpv-turn-head-left-fast"_bind, {
        {
            .keyWithMods = KeyCode::LEFT_ARROW | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl,
        },
        {
            .keyWithMods = KeyCode::LEFT_ARROW_PAD | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl,
        },
    });
    setBinds("fpv-turn-head-right-fast"_bind, {
        {
            .keyWithMods = KeyCode::RIGHT_ARROW | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl,
        },
        {
            .keyWithMods = KeyCode::RIGHT_ARROW_PAD | KeyModifier::Shift,
            .releasedModifiers = KeyModifier::Ctrl,
        },
    });

    setBinds("fpv-toggle-night-vision"_bind, { { KeyCode::KEY_N } });
    setBinds("fpv-exit"_bind, { { KeyCode::ESCAPE } });
    setBinds("fpv-menus"_bind, {
        {
            .keyWithMods = KeyCode::F10,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift | KeyModifier::Alt,
        },
    });

    setBinds("fpv-command-select-next"_bind, {
        { KeyCode::HOME },
    });
    setBinds("fpv-command-select-previous"_bind, {
        { KeyCode::END },
    });
    setBinds("fpv-command-attack"_bind, {
        { KeyCode::DELETE },
    });
    setBinds("fpv-command-move"_bind, {
        { KeyCode::PAGE_DOWN },
    });
    setBinds("fpv-command-follow"_bind, {
        { KeyCode::INSERT },
    });

    setBinds("gfx-toggle-rendering"_bind, {
        { KeyCode::F10 | KeyModifier::Ctrl | KeyModifier::Shift },
    });
}

void InputRegistry::setWasdLayout()
{
    // Add Ctrl to normal WASD binds
    setBinds("commands-stand-ground"_bind, {
        { KeyCode::KEY_W | KeyModifier::Ctrl },
    });
    setBinds("commands-attack"_bind, {
        { KeyCode::KEY_A | KeyModifier::Ctrl },
    });
    setBinds("commands-stop"_bind, {
        { KeyCode::KEY_S | KeyModifier::Ctrl },
    });
    setBinds("commands-deconstruct"_bind, {
        { KeyCode::KEY_X },
        { KeyCode::KEY_D | KeyModifier::Ctrl },
    });
    setBinds("commands-deploy"_bind, {
        { KeyCode::KEY_E | KeyModifier::Ctrl },
    });
    setBinds("commands-recycle"_bind, {
        { KeyCode::KEY_Q | KeyModifier::Ctrl },
    });

    setBinds("view-next-machine"_bind, {}); // Alt+A
    setBinds("view-next-construction"_bind, {}); // Alt+S
    setBinds("select-all-machines"_bind, {}); // Shift+A
    setBinds("select-all-constructions"_bind, {}); // Shift+S
    setBinds("add-all-machines"_bind, {}); // Ctrl+Shift+A
    setBinds("add-all-constructions"_bind, {}); // Ctrl+Shift+S

    setBinds("zenith-camera-forward"_bind, {
        { .keyWithMods = KeyCode::KEY_W, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::UP_ARROW, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-backward"_bind, {
        { .keyWithMods = KeyCode::KEY_S, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::DOWN_ARROW, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-slide-left"_bind, {
        { .keyWithMods = KeyCode::KEY_A, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::LEFT_ARROW, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-slide-right"_bind, {
        { .keyWithMods = KeyCode::KEY_D, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::RIGHT_ARROW, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("zenith-camera-rotate-left"_bind, {
        { .keyWithMods = KeyCode::KEY_A | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt },
        { .keyWithMods = KeyCode::LEFT_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt },
    });
    setBinds("zenith-camera-rotate-right"_bind, {
        { .keyWithMods = KeyCode::KEY_D | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt },
        { .keyWithMods = KeyCode::RIGHT_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Alt },
    });

    setBinds("ground-camera-forward"_bind, {
        { .keyWithMods = KeyCode::KEY_W, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::UP_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("ground-camera-backward"_bind, {
        { .keyWithMods = KeyCode::KEY_S, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
        { .keyWithMods = KeyCode::DOWN_ARROW, .releasedModifiers = KeyModifier::Alt | KeyModifier::Shift },
    });
    setBinds("ground-camera-slide-left"_bind, {
        { .keyWithMods = KeyCode::KEY_A, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::DELETE, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-slide-right"_bind, {
        { .keyWithMods = KeyCode::KEY_D, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::END, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-rotate-left"_bind, {
        { .keyWithMods = KeyCode::KEY_Q, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::LEFT_ARROW, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-rotate-right"_bind, {
        { .keyWithMods = KeyCode::KEY_E, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::RIGHT_ARROW, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-pitch-up"_bind, {
        { .keyWithMods = KeyCode::KEY_W | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::UP_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
    });
    setBinds("ground-camera-pitch-down"_bind, {
        { .keyWithMods = KeyCode::KEY_S | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
        { .keyWithMods = KeyCode::DOWN_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Alt },
    });

    setBinds("fpv-move-forward"_bind, {
        { .keyWithMods = KeyCode::KEY_W },
        { .keyWithMods = KeyCode::UP_ARROW },
    });
    setBinds("fpv-move-backward"_bind, {
        { .keyWithMods = KeyCode::KEY_S },
        { .keyWithMods = KeyCode::DOWN_ARROW },
    });

    setBinds("fpv-turn-left"_bind, {
        {
            .keyWithMods = KeyCode::KEY_A | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::LEFT_ARROW | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
    });
    setBinds("fpv-turn-right"_bind, {
        {
            .keyWithMods = KeyCode::KEY_D | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::RIGHT_ARROW | KeyModifier::Ctrl,
            .releasedModifiers = KeyModifier::Shift,
        },
    });

    setBinds("fpv-turn-left-fast"_bind, {
        {
            .keyWithMods = KeyCode::KEY_A,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::LEFT_ARROW,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });
    setBinds("fpv-turn-right-fast"_bind, {
        {
            .keyWithMods = KeyCode::KEY_D,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
        {
            .keyWithMods = KeyCode::RIGHT_ARROW,
            .releasedModifiers = KeyModifier::Ctrl | KeyModifier::Shift,
        },
    });

    setBinds("fpv-look-down-fast"_bind, {
        { .keyWithMods = KeyCode::KEY_W | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl },
        { .keyWithMods = KeyCode::UP_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl },
    });
    setBinds("fpv-look-up-fast"_bind, {
        { .keyWithMods = KeyCode::KEY_S | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl },
        { .keyWithMods = KeyCode::DOWN_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl },
    });
    setBinds("fpv-turn-head-left-fast"_bind, {
        { .keyWithMods = KeyCode::KEY_A | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl, },
        { .keyWithMods = KeyCode::LEFT_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl, },
    });
    setBinds("fpv-turn-head-right-fast"_bind, {
        { .keyWithMods = KeyCode::KEY_D | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl, },
        { .keyWithMods = KeyCode::RIGHT_ARROW | KeyModifier::Shift, .releasedModifiers = KeyModifier::Ctrl, },
    });
}

IInputRegistry* inputRegistry()
{
    return inputRegistryImpl();
}

InputRegistry *inputRegistryImpl()
{
    static BindsStorageXml storage;
    static InputRegistry r(&storage);
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
        r.save();
    }
    return &r;
}

} // namespace MachGui
