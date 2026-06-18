#include "PCH.h"
#include "EventHandler.h"
#include "EquipmentMenu.h"
#include "EquipmentSelectionMenu.h"
#include "Settings.h"

namespace NPCEquipmentViewer
{
    EventHandler& EventHandler::GetSingleton()
    {
        static EventHandler singleton;
        return singleton;
    }

    void EventHandler::Register()
    {
        auto& singleton = GetSingleton();

        if (auto* crosshairSource = SKSE::GetCrosshairRefEventSource(); crosshairSource != nullptr) {
            crosshairSource->AddEventSink(std::addressof(singleton));
        }

        if (auto* inputManager = RE::BSInputDeviceManager::GetSingleton(); inputManager != nullptr) {
            inputManager->AddEventSink(std::addressof(singleton));
        }
    }

    RE::BSEventNotifyControl EventHandler::ProcessEvent(
        const SKSE::CrosshairRefEvent* event,
        RE::BSTEventSource<SKSE::CrosshairRefEvent>*)
    {
        std::scoped_lock lock(targetMutex_);
        crosshairTarget_ = event != nullptr ? event->crosshairRef : nullptr;
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl EventHandler::ProcessEvent(
        RE::InputEvent* const* event,
        RE::BSTEventSource<RE::InputEvent*>*)
    {
        if (event == nullptr || *event == nullptr) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto& settings = Settings::GetSingleton();

        for (auto* currentEvent = *event;
             currentEvent != nullptr;
             currentEvent = currentEvent->next) {
            if (EquipmentSelectionMenu::IsOpen()) {
                EquipmentSelectionMenu::HandleNavigationInput(currentEvent);
                continue;
            }

            if (currentEvent->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) {
                continue;
            }

            auto* buttonEvent = currentEvent->AsButtonEvent();
            if (buttonEvent == nullptr || !buttonEvent->IsDown()) {
                continue;
            }

            if (buttonEvent->GetDevice() != RE::INPUT_DEVICE::kKeyboard) {
                continue;
            }

            if (buttonEvent->GetIDCode() != settings.GetKeyCode()) {
                continue;
            }

            if (auto* ui = RE::UI::GetSingleton(); ui != nullptr && ui->GameIsPaused()) {
                continue;
            }

            auto target = GetCrosshairTarget();
            if (!target) {
                if (auto* player = RE::PlayerCharacter::GetSingleton(); player != nullptr) {
                    target.reset(player);
                    SKSE::log::info(
                        "[MenuDiagnostic] No crosshair target; using player equipment");
                }
            }

            if (const auto* taskInterface = SKSE::GetTaskInterface();
                taskInterface != nullptr) {
                taskInterface->AddTask([target = std::move(target)]() {
                    EquipmentMenu::Show(target);
                });
            }

            break;
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    RE::NiPointer<RE::TESObjectREFR> EventHandler::GetCrosshairTarget()
    {
        std::scoped_lock lock(targetMutex_);
        return crosshairTarget_;
    }
}
