#pragma once

namespace NPCEquipmentViewer
{
    class MessageBox final
    {
    public:
        using Callback = std::function<void(std::uint32_t)>;

        static bool Show(
            const std::string& body,
            const std::vector<std::string>& buttons,
            Callback callback);
    };
}
