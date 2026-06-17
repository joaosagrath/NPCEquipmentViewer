#pragma once

namespace NPCEquipmentViewer
{
    class Settings final
    {
    public:
        static Settings& GetSingleton();

        void Load();

        [[nodiscard]] std::uint32_t GetKeyCode() const noexcept;
        [[nodiscard]] bool ShowFormID() const noexcept;
        [[nodiscard]] bool ShowSlots() const noexcept;
        [[nodiscard]] bool ShowItemType() const noexcept;

    private:
        Settings() = default;

        std::uint32_t keyCode_{ 0x23 };  // DIK_H
        bool showFormID_{ false };
        bool showSlots_{ true };
        bool showItemType_{ false };
    };
}
