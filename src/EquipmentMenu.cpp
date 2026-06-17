#include "PCH.h"
#include "EquipmentMenu.h"
#include "KidWriter.h"
#include "MessageBox.h"
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
        std::size_t page{ 0 };
    };

    constexpr std::size_t kItemsPerPage = 6;

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

    std::string GetFormName(RE::TESForm* form, RE::InventoryEntryData* entryData = nullptr)
    {
        if (entryData != nullptr) {
            if (const auto* displayName = entryData->GetDisplayName(); displayName != nullptr && displayName[0] != '\0') {
                return displayName;
            }
        }

        if (form != nullptr) {
            if (const auto* name = form->GetName(); name != nullptr && name[0] != '\0') {
                return name;
            }

            if (const auto* editorID = form->GetFormEditorID(); editorID != nullptr && editorID[0] != '\0') {
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

        if (const auto* displayName = actor->GetDisplayFullName(); displayName != nullptr && displayName[0] != '\0') {
            return displayName;
        }

        if (const auto* name = actor->GetName(); name != nullptr && name[0] != '\0') {
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
        if (armor->HasPartOf(BipedSlot::kHands) || armor->HasPartOf(BipedSlot::kForearms)) {
            return "Hands";
        }
        if (armor->HasPartOf(BipedSlot::kFeet) || armor->HasPartOf(BipedSlot::kCalves)) {
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
                output << ", ";
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
        auto* armor = form != nullptr ? form->As<RE::TESObjectARMO>() : nullptr;
        if (armor == nullptr) {
            return;
        }

        const auto duplicate = std::find_if(items.begin(), items.end(), [armor](const EquipmentItem& item) {
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

    std::string BuildButtonLabel(const EquipmentItem& item)
    {
        const auto& settings = NPCEquipmentViewer::Settings::GetSingleton();
        std::ostringstream output;
        output << item.displayName;

        if (settings.ShowItemType()) {
            output << " [" << item.type << ']';
        }

        if (settings.ShowSlots() && !item.slots.empty()) {
            output << " | Slots: " << item.slots;
        }

        if (settings.ShowFormID()) {
            output << " | " << FormatFormID(item.armor);
        }

        return output.str();
    }

    void NotifyWriteResult(const NPCEquipmentViewer::KidWriter::WriteResult& result)
    {
        using Result = NPCEquipmentViewer::KidWriter::Result;

        switch (result.result) {
        case Result::kAdded:
            RE::DebugNotification("Item added to Custom_modesty_KID.ini. Restart Skyrim to apply it.");
            break;
        case Result::kDuplicate:
            RE::DebugNotification("This item is already present in Custom_modesty_KID.ini.");
            break;
        case Result::kInvalidArmor:
            RE::DebugNotification("The selected item cannot be written as an Armor rule.");
            break;
        case Result::kFileError:
        default:
            RE::DebugNotification("Could not write Custom_modesty_KID.ini.");
            break;
        }
    }

    void ShowMenuPage(const std::shared_ptr<MenuState>& state)
    {
        if (!state || state->items.empty()) {
            return;
        }

        const auto pageCount = (state->items.size() + kItemsPerPage - 1) / kItemsPerPage;
        if (state->page >= pageCount) {
            state->page = pageCount - 1;
        }

        const auto firstItem = state->page * kItemsPerPage;
        const auto lastItem = std::min(firstItem + kItemsPerPage, state->items.size());
        const auto visibleItemCount = lastItem - firstItem;
        const bool hasPreviousPage = state->page > 0;
        const bool hasNextPage = state->page + 1 < pageCount;

        std::ostringstream body;
        body << "Equipment worn by " << state->actorName << "\n\n"
             << "Select an armor, clothing, shield, or accessory to add it to "
             << "Custom_modesty_KID.ini.\n"
             << "Use the keyboard or controller to navigate.\n\n"
             << "Page " << (state->page + 1) << " of " << pageCount;

        std::vector<std::string> buttons;
        buttons.reserve(visibleItemCount + 3);

        for (auto index = firstItem; index < lastItem; ++index) {
            buttons.push_back(BuildButtonLabel(state->items[index]));
        }

        if (hasPreviousPage) {
            buttons.emplace_back("Previous page");
        }
        if (hasNextPage) {
            buttons.emplace_back("Next page");
        }
        buttons.emplace_back("Close");

        const bool opened = NPCEquipmentViewer::MessageBox::Show(
            body.str(),
            buttons,
            [state, firstItem, visibleItemCount, hasPreviousPage, hasNextPage](const std::uint32_t selectedIndex) {
                if (selectedIndex < visibleItemCount) {
                    const auto& item = state->items[firstItem + selectedIndex];
                    const auto result = NPCEquipmentViewer::KidWriter::AddArmor(item.armor, item.displayName);
                    NotifyWriteResult(result);
                    return;
                }

                auto commandIndex = static_cast<std::size_t>(selectedIndex - visibleItemCount);

                if (hasPreviousPage) {
                    if (commandIndex == 0) {
                        --state->page;
                        ShowMenuPage(state);
                        return;
                    }
                    --commandIndex;
                }

                if (hasNextPage && commandIndex == 0) {
                    ++state->page;
                    ShowMenuPage(state);
                }
            });

        if (!opened) {
            RE::DebugNotification("Could not open the equipment selection menu.");
        }
    }
}

namespace NPCEquipmentViewer
{
    void EquipmentMenu::Show(const RE::NiPointer<RE::TESObjectREFR>& target)
    {
        if (!target) {
            RE::DebugNotification("No target under the crosshair.");
            return;
        }

        auto* actor = target->As<RE::Actor>();
        if (actor == nullptr) {
            RE::DebugNotification("The crosshair target is not an NPC.");
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

            if (object == nullptr || entryData == nullptr || count <= 0 || !entryData->IsWorn()) {
                continue;
            }

            AddArmorItem(state->items, object, entryData.get());
        }

        std::sort(state->items.begin(), state->items.end(), [](const EquipmentItem& left, const EquipmentItem& right) {
            if (left.type != right.type) {
                return left.type < right.type;
            }
            return left.displayName < right.displayName;
        });

        if (state->items.empty()) {
            RE::DebugNotification("No equipped armor or clothing was detected.");
            return;
        }

        ShowMenuPage(state);
    }
}
