#pragma once

namespace NPCEquipmentViewer
{
    class UIExtensionsMenu final
    {
    public:
        using SelectionCallback = std::function<void(std::int32_t)>;

        [[nodiscard]] static bool IsAvailable();

        static bool Show(
            const std::vector<std::string>& entries,
            SelectionCallback callback);
    };
}
