#pragma once

namespace NPCEquipmentViewer
{
    class EquipmentSelectionMenu final
    {
    public:
        using SelectionCallback = std::function<void(std::size_t)>;

        static void Register();

        [[nodiscard]] static bool Show(
            std::string title,
            std::vector<std::string> entries,
            SelectionCallback callback);

        [[nodiscard]] static bool IsOpen();
        [[nodiscard]] static bool HandleNavigationInput(
            RE::InputEvent* event);
    };
}
