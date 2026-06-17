#include "PCH.h"
#include "EquipmentMenu.h"
#include "Settings.h"

namespace
{
    using BipedSlot = RE::BGSBipedObjectForm::BipedObjectSlot;

    struct SlotDescription
    {
        BipedSlot slot;
        std::uint32_t number;
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

    std::string FormatFormID(const RE::TESForm* form)
    {
        if (form == nullptr) {
            return {};
        }

        std::ostringstream output;
        output << "0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << form->GetFormID();
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
            if (const auto* editorID = form->GetFormEditorID(); editorID != nullptr && editorID[0] != '\0') {
                return editorID;
            }
        }

        return "Item sem nome";
    }

        std::string GetArmorType(RE::TESObjectARMO* armor)
    {
        if (armor == nullptr) {
            return "Equipamento";
        }

        if (armor->IsShield()) {
            return "Escudo";
        }

        if (armor->HasPartOf(BipedSlot::kCirclet)) {
            return "Tiara";
        }

        if (armor->HasPartOf(BipedSlot::kHead)) {
            return "Cabeca";
        }

        if (armor->HasPartOf(BipedSlot::kBody)) {
            return armor->IsClothing() ? "Roupa" : "Corpo";
        }

        if (armor->HasPartOf(BipedSlot::kHands) ||
            armor->HasPartOf(BipedSlot::kForearms)) {
            return "Maos";
        }

        if (armor->HasPartOf(BipedSlot::kFeet) ||
            armor->HasPartOf(BipedSlot::kCalves)) {
            return "Pes";
        }

        if (armor->HasPartOf(BipedSlot::kAmulet) ||
            armor->HasPartOf(BipedSlot::kRing) ||
            armor->HasPartOf(BipedSlot::kEars) ||
            armor->HasPartOf(BipedSlot::kModMouth) ||
            armor->HasPartOf(BipedSlot::kModNeck) ||
            armor->HasPartOf(BipedSlot::kModFaceJewelry)) {
            return "Acessorio";
        }

        if (armor->IsClothing()) {
            return "Roupa";
        }

        if (armor->IsHeavyArmor()) {
            return "Armadura pesada";
        }

        if (armor->IsLightArmor()) {
            return "Armadura leve";
        }

        return "Armadura";
    }
    
    std::string GetItemType(RE::TESForm* form)
    {
        if (form == nullptr) {
            return "Equipamento";
        }

        if (auto* armor = form->As<RE::TESObjectARMO>(); armor != nullptr) {
            return GetArmorType(armor);
        }
        if (form->As<RE::TESObjectWEAP>() != nullptr) {
            return "Arma";
        }
        if (form->As<RE::TESAmmo>() != nullptr) {
            return "Municao";
        }
        if (form->As<RE::SpellItem>() != nullptr) {
            return "Magia";
        }
        if (form->As<RE::ScrollItem>() != nullptr) {
            return "Pergaminho";
        }
        if (form->As<RE::TESObjectLIGH>() != nullptr) {
            return "Tocha";
        }

        return "Equipamento";
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

    std::string BuildItemLine(
        const std::string_view location,
        RE::TESForm* form,
        RE::InventoryEntryData* entryData,
        const NPCEquipmentViewer::Settings& settings)
    {
        std::ostringstream output;
        output << "- [" << location << "] " << GetFormName(form, entryData);

        if (settings.ShowItemType()) {
            output << " (" << GetItemType(form) << ")";
        }

        if (settings.ShowSlots()) {
            if (auto* armor = form != nullptr ? form->As<RE::TESObjectARMO>() : nullptr; armor != nullptr) {
                const auto slots = GetArmorSlots(armor);
                if (!slots.empty()) {
                    output << " | Slots: " << slots;
                }
            }
        }

        if (settings.ShowFormID() && form != nullptr) {
            output << " | " << FormatFormID(form);
        }

        return output.str();
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

        return "NPC sem nome";
    }
}

namespace NPCEquipmentViewer
{
    void EquipmentMenu::Show(const RE::NiPointer<RE::TESObjectREFR>& target)
    {
        if (!target) {
            RE::DebugNotification("Nenhum alvo no crosshair.");
            return;
        }

        auto* actor = target->As<RE::Actor>();
        if (actor == nullptr) {
            RE::DebugNotification("O alvo no crosshair nao e um NPC.");
            return;
        }

        const auto& settings = Settings::GetSingleton();
        std::vector<std::string> lines;
        lines.reserve(40);

        auto* rightHand = actor->GetEquippedObject(false);
        auto* leftHand = actor->GetEquippedObject(true);

        if (rightHand != nullptr) {
            lines.push_back(BuildItemLine("Mao direita", rightHand, actor->GetEquippedEntryData(false), settings));
        }

        if (leftHand != nullptr) {
            lines.push_back(BuildItemLine("Mao esquerda", leftHand, actor->GetEquippedEntryData(true), settings));
        }

        const auto handEntryCount = lines.size();

        auto inventory = actor->GetInventory();
        for (auto& [object, inventoryData] : inventory) {
            auto& [count, entryData] = inventoryData;

            if (object == nullptr || entryData == nullptr || count <= 0 || !entryData->IsWorn()) {
                continue;
            }

            if (object == rightHand || object == leftHand) {
                continue;
            }

            const auto location = GetItemType(object);
            lines.push_back(BuildItemLine(location, object, entryData.get(), settings));
        }

        const auto firstInventoryEntry = lines.begin() + static_cast<std::vector<std::string>::difference_type>(handEntryCount);
        std::sort(firstInventoryEntry, lines.end());

        std::ostringstream message;
        message << "Equipamento de " << GetActorName(actor);

        if (settings.ShowFormID()) {
            message << " (" << FormatFormID(actor) << ")";
        }

        message << "\n\n";

        if (lines.empty()) {
            message << "Nenhum equipamento detectado.";
        } else {
            constexpr std::size_t maximumEntries = 48;
            const auto displayedEntries = std::min(lines.size(), maximumEntries);

            for (std::size_t index = 0; index < displayedEntries; ++index) {
                message << lines[index] << '\n';
            }

            if (lines.size() > maximumEntries) {
                message << "\n... " << (lines.size() - maximumEntries) << " item(ns) omitido(s).";
            }
        }

        RE::DebugMessageBox(message.str().c_str());
    }
}
