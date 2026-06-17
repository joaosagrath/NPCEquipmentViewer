#pragma once

namespace NPCEquipmentViewer
{
    class EventHandler final :
        public RE::BSTEventSink<SKSE::CrosshairRefEvent>,
        public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static EventHandler& GetSingleton();
        static void Register();

        RE::BSEventNotifyControl ProcessEvent(
            const SKSE::CrosshairRefEvent* event,
            RE::BSTEventSource<SKSE::CrosshairRefEvent>* eventSource) override;

        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* event,
            RE::BSTEventSource<RE::InputEvent*>* eventSource) override;

    private:
        EventHandler() = default;

        [[nodiscard]] RE::NiPointer<RE::TESObjectREFR> GetCrosshairTarget();

        std::mutex targetMutex_;
        RE::NiPointer<RE::TESObjectREFR> crosshairTarget_;
    };
}
