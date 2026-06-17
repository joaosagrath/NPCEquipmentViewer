#include "PCH.h"
#include "KidWriter.h"

namespace
{
    constexpr std::string_view kOutputFileName = "Custom_modesty_KID.ini";
    constexpr std::string_view kKeywordName = "NoModestyAll";
    std::mutex g_fileMutex;

    std::string Trim(std::string value)
    {
        const auto notSpace = [](const unsigned char character) {
            return std::isspace(character) == 0;
        };

        value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
        value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
        return value;
    }

    std::string Normalize(std::string value)
    {
        value = Trim(std::move(value));
        std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    std::string SanitizeFilter(std::string value)
    {
        std::replace(value.begin(), value.end(), '|', ' ');
        std::replace(value.begin(), value.end(), '\r', ' ');
        std::replace(value.begin(), value.end(), '\n', ' ');
        return Trim(std::move(value));
    }

    std::filesystem::path GetDataDirectory()
    {
        std::array<wchar_t, 32768> executablePath{};
        const auto length = GetModuleFileNameW(
            nullptr,
            executablePath.data(),
            static_cast<DWORD>(executablePath.size()));

        if (length > 0 && length < executablePath.size()) {
            return std::filesystem::path(executablePath.data()).parent_path() / "Data";
        }

        return std::filesystem::current_path() / "Data";
    }

    std::string BuildExactFilter(RE::TESObjectARMO* armor)
    {
        if (armor == nullptr || armor->IsDynamicForm()) {
            return {};
        }

        const auto* sourceFile = armor->GetFile(0);
        if (sourceFile == nullptr || sourceFile->GetFilename().empty()) {
            return {};
        }

        std::ostringstream output;
        output << "0x"
               << std::uppercase
               << std::hex
               << std::setw(6)
               << std::setfill('0')
               << armor->GetLocalFormID()
               << '~'
               << sourceFile->GetFilename();
        return output.str();
    }

    std::string BuildFallbackFilter(RE::TESObjectARMO* armor, const std::string& displayName)
    {
        if (armor != nullptr) {
            if (const auto* editorID = armor->GetFormEditorID(); editorID != nullptr && editorID[0] != '\0') {
                return SanitizeFilter(editorID);
            }

            if (const auto* name = armor->GetName(); name != nullptr && name[0] != '\0') {
                return SanitizeFilter(name);
            }
        }

        return SanitizeFilter(displayName);
    }

    std::string BuildLine(const std::string& filter)
    {
        return "Keyword = " + std::string(kKeywordName) + "|Armor|" + filter;
    }

    bool ContainsLine(const std::string& content, const std::vector<std::string>& candidates)
    {
        std::istringstream input(content);
        std::string existingLine;

        while (std::getline(input, existingLine)) {
            const auto normalizedExisting = Normalize(existingLine);

            for (const auto& candidate : candidates) {
                if (normalizedExisting == Normalize(candidate)) {
                    return true;
                }
            }
        }

        return false;
    }
}

namespace NPCEquipmentViewer
{
    KidWriter::WriteResult KidWriter::AddArmor(
        RE::TESObjectARMO* armor,
        const std::string& displayName)
    {
        WriteResult result;
        result.path = GetOutputPath();

        if (armor == nullptr) {
            result.result = Result::kInvalidArmor;
            return result;
        }

        const auto exactFilter = BuildExactFilter(armor);
        const auto fallbackFilter = BuildFallbackFilter(armor, displayName);
        const auto selectedFilter = !exactFilter.empty() ? exactFilter : fallbackFilter;

        if (selectedFilter.empty()) {
            result.result = Result::kInvalidArmor;
            return result;
        }

        result.line = BuildLine(selectedFilter);
        std::vector<std::string> duplicateCandidates{ result.line };

        if (!fallbackFilter.empty()) {
            duplicateCandidates.push_back(BuildLine(fallbackFilter));
        }

        std::scoped_lock lock(g_fileMutex);
        std::string existingContent;

        if (std::filesystem::exists(result.path)) {
            std::ifstream input(result.path, std::ios::binary);
            if (!input.is_open()) {
                result.result = Result::kFileError;
                return result;
            }

            existingContent.assign(
                std::istreambuf_iterator<char>(input),
                std::istreambuf_iterator<char>());

            if (ContainsLine(existingContent, duplicateCandidates)) {
                result.result = Result::kDuplicate;
                return result;
            }
        }

        std::ofstream output(result.path, std::ios::binary | std::ios::app);
        if (!output.is_open()) {
            result.result = Result::kFileError;
            return result;
        }

        if (!existingContent.empty() && existingContent.back() != '\n') {
            output << '\n';
        }

        output << result.line << '\n';
        output.flush();

        result.result = output.good() ? Result::kAdded : Result::kFileError;
        return result;
    }

    std::filesystem::path KidWriter::GetOutputPath()
    {
        return GetDataDirectory() / kOutputFileName;
    }
}
