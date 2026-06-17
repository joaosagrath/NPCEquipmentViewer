#pragma once

namespace NPCEquipmentViewer
{
    class KidWriter final
    {
    public:
        enum class Result
        {
            kAdded,
            kDuplicate,
            kInvalidArmor,
            kFileError
        };

        struct WriteResult
        {
            Result result{ Result::kFileError };
            std::string line;
            std::filesystem::path path;
        };

        static WriteResult AddArmor(
            RE::TESObjectARMO* armor,
            const std::string& displayName);

        [[nodiscard]] static std::filesystem::path GetOutputPath();
    };
}
