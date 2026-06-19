#include "PCH.h"
#include "EquipmentMenu.h"
#include "EquipmentSelectionMenu.h"
#include "KidWriter.h"
#include "Settings.h"

namespace
{
    using BipedSlot = RE::BGSBipedObjectForm::BipedObjectSlot;

    struct SlotDescription
    {
        BipedSlot slot;
        std::uint32_t number;
    };

    struct EquipmentItem
    {
        RE::TESObjectARMO* armor{ nullptr };
        std::string displayName;
        std::string type;
        std::string slots;
    };

    struct MenuState
    {
        std::string actorName;
        std::vector<EquipmentItem> items;
    };

    struct KeywordOption
    {
        std::string_view keyword;
        std::string_view description;
    };

    constexpr std::array<SlotDescription, 32> kBipedSlots{
        SlotDescription{ BipedSlot::kHead, 30 },
        SlotDescription{ BipedSlot::kHair, 31 },
        SlotDescription{ BipedSlot::kBody, 32 },
        SlotDescription{ BipedSlot::kHands, 33 },
        SlotDescription{ BipedSlot::kForearms, 34 },
        SlotDescription{ BipedSlot::kAmulet, 35 },
        SlotDescription{ BipedSlot::kRing, 36 },
        SlotDescription{ BipedSlot::kFeet, 37 },
        SlotDescription{ BipedSlot::kCalves, 38 },
        SlotDescription{ BipedSlot::kShield, 39 },
        SlotDescription{ BipedSlot::kTail, 40 },
        SlotDescription{ BipedSlot::kLongHair, 41 },
        SlotDescription{ BipedSlot::kCirclet, 42 },
        SlotDescription{ BipedSlot::kEars, 43 },
        SlotDescription{ BipedSlot::kModMouth, 44 },
        SlotDescription{ BipedSlot::kModNeck, 45 },
        SlotDescription{ BipedSlot::kModChestPrimary, 46 },
        SlotDescription{ BipedSlot::kModBack, 47 },
        SlotDescription{ BipedSlot::kModMisc1, 48 },
        SlotDescription{ BipedSlot::kModPelvisPrimary, 49 },
        SlotDescription{ BipedSlot::kDecapitateHead, 50 },
        SlotDescription{ BipedSlot::kDecapitate, 51 },
        SlotDescription{ BipedSlot::kModPelvisSecondary, 52 },
        SlotDescription{ BipedSlot::kModLegRight, 53 },
        SlotDescription{ BipedSlot::kModLegLeft, 54 },
        SlotDescription{ BipedSlot::kModFaceJewelry, 55 },
        SlotDescription{ BipedSlot::kModChestSecondary, 56 },
        SlotDescription{ BipedSlot::kModShoulder, 57 },
        SlotDescription{ BipedSlot::kModArmLeft, 58 },
        SlotDescription{ BipedSlot::kModArmRight, 59 },
        SlotDescription{ BipedSlot::kModMisc2, 60 },
        SlotDescription{ BipedSlot::kFX01, 61 }
    };

    constexpr std::array<KeywordOption, 5> kKeywordOptions{
        KeywordOption{ "Modesty", "top and bottom exposed" },
        KeywordOption{ "NoModesty", "nothing exposed" },
        KeywordOption{ "NoModestyTop", "top covered" },
        KeywordOption{ "NoModestyBottom", "bottom covered" },
        KeywordOption{ "NoModestyAll", "fully covered" }
    };

    std::string FormatFormID(const RE::TESForm* form)
    {
        if (form == nullptr) {
            return {};
        }

        std::ostringstream output;
        output << "0x"
               << std::uppercase
               << std::hex
               << std::setw(8)
               << std::setfill('0')
               << form->GetFormID();
        return output.str();
    }

    std::string GetFormName(
        RE::TESForm* form,
        RE::InventoryEntryData* entryData = nullptr)
    {
        if (entryData != nullptr) {
            if (const auto* displayName = entryData->GetDisplayName();
                displayName != nullptr && displayName[0] != '\0') {
                return displayName;
            }
        }

        if (form != nullptr) {
            if (const auto* name = form->GetName();
                name != nullptr && name[0] != '\0') {
                return name;
            }

            if (const auto* editorID = form->GetFormEditorID();
                editorID != nullptr && editorID[0] != '\0') {
                return editorID;
            }
        }

        return "Unnamed item";
    }

    std::string GetActorName(RE::Actor* actor)
    {
        if (actor == nullptr) {
            return "NPC";
        }

        if (const auto* displayName = actor->GetDisplayFullName();
            displayName != nullptr && displayName[0] != '\0') {
            return displayName;
        }

        if (const auto* name = actor->GetName();
            name != nullptr && name[0] != '\0') {
            return name;
        }

        return "Unnamed NPC";
    }

    std::string GetArmorType(RE::TESObjectARMO* armor)
    {
        if (armor == nullptr) {
            return "Equipment";
        }

        if (armor->IsShield()) {
            return "Shield";
        }
        if (armor->HasPartOf(BipedSlot::kCirclet)) {
            return "Circlet";
        }
        if (armor->HasPartOf(BipedSlot::kHead)) {
            return "Head";
        }
        if (armor->HasPartOf(BipedSlot::kBody)) {
            return armor->IsClothing() ? "Clothing" : "Body armor";
        }
        if (armor->HasPartOf(BipedSlot::kHands) ||
            armor->HasPartOf(BipedSlot::kForearms)) {
            return "Hands";
        }
        if (armor->HasPartOf(BipedSlot::kFeet) ||
            armor->HasPartOf(BipedSlot::kCalves)) {
            return "Feet";
        }
        if (armor->HasPartOf(BipedSlot::kAmulet) ||
            armor->HasPartOf(BipedSlot::kRing) ||
            armor->HasPartOf(BipedSlot::kEars) ||
            armor->HasPartOf(BipedSlot::kModMouth) ||
            armor->HasPartOf(BipedSlot::kModNeck) ||
            armor->HasPartOf(BipedSlot::kModFaceJewelry)) {
            return "Accessory";
        }
        if (armor->IsClothing()) {
            return "Clothing";
        }
        if (armor->IsHeavyArmor()) {
            return "Heavy armor";
        }
        if (armor->IsLightArmor()) {
            return "Light armor";
        }

        return "Armor";
    }

    std::string GetArmorSlots(RE::TESObjectARMO* armor)
    {
        if (armor == nullptr) {
            return {};
        }

        std::ostringstream output;
        bool first = true;

        for (const auto& slot : kBipedSlots) {
            if (!armor->HasPartOf(slot.slot)) {
                continue;
            }

            if (!first) {
                output << ',';
            }

            output << slot.number;
            first = false;
        }

        return output.str();
    }

    void AddArmorItem(
        std::vector<EquipmentItem>& items,
        RE::TESForm* form,
        RE::InventoryEntryData* entryData)
    {
        auto* armor =
            form != nullptr ? form->As<RE::TESObjectARMO>() : nullptr;
        if (armor == nullptr) {
            return;
        }

        const auto duplicate = std::find_if(
            items.begin(),
            items.end(),
            [armor](const EquipmentItem& item) {
                return item.armor == armor;
            });

        if (duplicate != items.end()) {
            return;
        }

        items.push_back(EquipmentItem{
            armor,
            GetFormName(form, entryData),
            GetArmorType(armor),
            GetArmorSlots(armor)
        });
    }

    std::string BuildListLabel(const EquipmentItem& item)
    {
        const auto& settings =
            NPCEquipmentViewer::Settings::GetSingleton();

        std::ostringstream output;
        output << item.displayName;

        if (settings.ShowItemType()) {
            output << " [" << item.type << ']';
        }

        if (settings.ShowSlots() && !item.slots.empty()) {
            output << " | Slot:" << item.slots;
        }

        if (settings.ShowFormID()) {
            output << " | " << FormatFormID(item.armor);
        }

        return output.str();
    }

    void NotifyWriteResult(
        const NPCEquipmentViewer::KidWriter::WriteResult& result)
    {
        using Result = NPCEquipmentViewer::KidWriter::Result;

        switch (result.result) {
        case Result::kAdded:
            RE::DebugNotification(
                "Item added to Custom_modesty_KID.ini. "
                "Restart Skyrim to apply it.");
            break;

        case Result::kUpdated:
            RE::DebugNotification(
                "Item keyword updated in Custom_modesty_KID.ini. "
                "Restart Skyrim to apply it.");
            break;

        case Result::kDuplicate:
            RE::DebugNotification(
                "This item is already present in "
                "Custom_modesty_KID.ini.");
            break;

        case Result::kInvalidArmor:
            RE::DebugNotification(
                "The selected item cannot be written as an Armor rule.");
            break;

        case Result::kFileError:
        default:
            RE::DebugNotification(
                "Could not write Custom_modesty_KID.ini.");
            break;
        }
    }

    bool ArmorHasKeyword(
        const RE::TESObjectARMO* armor,
        const std::string_view keywordEditorID)
    {
        if (armor == nullptr ||
            armor->keywords == nullptr ||
            keywordEditorID.empty()) {
            return false;
        }

        for (std::uint32_t index = 0; index < armor->numKeywords; ++index) {
            const auto* keyword = armor->keywords[index];
            if (keyword == nullptr) {
                continue;
            }

            const auto* editorID = keyword->GetFormEditorID();
            if (editorID == nullptr) {
                continue;
            }

            if (std::string_view(editorID) == keywordEditorID) {
                return true;
            }
        }

        return false;
    }

    std::string BuildKeywordLabel(
        const KeywordOption& option,
        const bool isCurrent)
    {
        auto label = std::string(option.keyword) + " | " +
                     std::string(option.description);

        if (isCurrent) {
            label += " [CURRENT]";
        }

        return label;
    }

    void QueueGameTask(std::function<void()> task)
    {
        if (const auto* taskInterface = SKSE::GetTaskInterface();
            taskInterface != nullptr) {
            taskInterface->AddTask(std::move(task));
        }
    }

    void ShowKeywordSelectionMenu(
        const std::shared_ptr<MenuState>& state,
        const std::size_t itemIndex)
    {
        if (!state || itemIndex >= state->items.size()) {
            return;
        }

        std::vector<std::string> entries;
        entries.reserve(kKeywordOptions.size());

        const auto* armor = state->items[itemIndex].armor;

        for (const auto& option : kKeywordOptions) {
            entries.push_back(
                BuildKeywordLabel(
                    option,
                    ArmorHasKeyword(armor, option.keyword)));
        }

        const auto title =
            "Select keyword for " + state->items[itemIndex].displayName;

        const bool opened =
            NPCEquipmentViewer::EquipmentSelectionMenu::Show(
                title,
                std::move(entries),
                [state, itemIndex](const std::size_t selectedIndex) {
                    if (itemIndex >= state->items.size() ||
                        selectedIndex >= kKeywordOptions.size()) {
                        return;
                    }

                    QueueGameTask([state, itemIndex, selectedIndex]() {
                        const auto& item = state->items[itemIndex];
                        const auto& keyword = kKeywordOptions[selectedIndex];
                        const auto result =
                            NPCEquipmentViewer::KidWriter::AddArmor(
                                item.armor,
                                item.displayName,
                                keyword.keyword,
                                keyword.description);
                        NotifyWriteResult(result);
                    });
                });

        if (!opened) {
            RE::DebugNotification(
                "Could not open the keyword selection menu.");
        }
    }

    void ShowEquipmentSelectionMenu(
        const std::shared_ptr<MenuState>& state)
    {
        if (!state || state->items.empty()) {
            return;
        }

        std::vector<std::string> entries;
        entries.reserve(state->items.size());

        for (const auto& item : state->items) {
            entries.push_back(BuildListLabel(item));
        }

        const auto title =
            "Equipment worn by " + state->actorName;

        const bool opened =
            NPCEquipmentViewer::EquipmentSelectionMenu::Show(
                title,
                std::move(entries),
                [state](const std::size_t selectedIndex) {
                    if (selectedIndex >= state->items.size()) {
                        return;
                    }

                    QueueGameTask([state, selectedIndex]() {
                        ShowKeywordSelectionMenu(state, selectedIndex);
                    });
                });

        if (!opened) {
            RE::DebugNotification(
                "Could not open the equipment selection menu.");
        }
    }
}

namespace NPCEquipmentViewer
{
    void EquipmentMenu::Show(
        const RE::NiPointer<RE::TESObjectREFR>& target)
    {
        if (!target) {
            RE::DebugNotification(
                "No target under the crosshair.");
            return;
        }

        auto* actor = target->As<RE::Actor>();
        if (actor == nullptr) {
            RE::DebugNotification(
                "The crosshair target is not an NPC.");
            return;
        }

        auto state = std::make_shared<MenuState>();
        state->actorName = GetActorName(actor);
        state->items.reserve(32);

        AddArmorItem(
            state->items,
            actor->GetEquippedObject(false),
            actor->GetEquippedEntryData(false));
        AddArmorItem(
            state->items,
            actor->GetEquippedObject(true),
            actor->GetEquippedEntryData(true));

        auto inventory = actor->GetInventory();
        for (auto& [object, inventoryData] : inventory) {
            auto& [count, entryData] = inventoryData;

            if (object == nullptr ||
                entryData == nullptr ||
                count <= 0 ||
                !entryData->IsWorn()) {
                continue;
            }

            AddArmorItem(
                state->items,
                object,
                entryData.get());
        }

        std::sort(
            state->items.begin(),
            state->items.end(),
            [](const EquipmentItem& left, const EquipmentItem& right) {
                if (left.type != right.type) {
                    return left.type < right.type;
                }

                return left.displayName < right.displayName;
            });

        if (state->items.empty()) {
            RE::DebugNotification(
                "No equipped armor or clothing was detected.");
            return;
        }

        ShowEquipmentSelectionMenu(state);
    }
}
