#include "PCH.h"
#include "KidWriter.h"

namespace
{
    constexpr std::string_view kOutputFileName = "Custom_modesty_KID.ini";
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

    std::string SanitizeComment(std::string value)
    {
        std::replace(value.begin(), value.end(), '|', '/');
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

    std::string FormatLocalFormID(const RE::TESForm* form)
    {
        if (form == nullptr) {
            return {};
        }

        auto formID = form->GetFormID();
        if (!form->IsDynamicForm() && form->GetFile(0) != nullptr) {
            formID = form->GetLocalFormID();
        }

        const auto minimumWidth = formID > 0x00FFFFFF ? 8 : 6;

        std::ostringstream output;
        output << "0x"
               << std::uppercase
               << std::hex
               << std::setw(minimumWidth)
               << std::setfill('0')
               << formID;
        return output.str();
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

        return FormatLocalFormID(armor) + "~" + std::string(sourceFile->GetFilename());
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

    std::string BuildCommentName(RE::TESObjectARMO* armor, const std::string& displayName)
    {
        auto name = SanitizeComment(displayName);
        if (!name.empty()) {
            return name;
        }

        if (armor != nullptr) {
            if (const auto* armorName = armor->GetName(); armorName != nullptr && armorName[0] != '\0') {
                name = SanitizeComment(armorName);
                if (!name.empty()) {
                    return name;
                }
            }

            if (const auto* editorID = armor->GetFormEditorID(); editorID != nullptr && editorID[0] != '\0') {
                name = SanitizeComment(editorID);
                if (!name.empty()) {
                    return name;
                }
            }
        }

        return "Unnamed armor";
    }

    std::string BuildCommentLine(RE::TESObjectARMO* armor, const std::string& displayName)
    {
        return "; " + BuildCommentName(armor, displayName) +
               " | ID:" + FormatLocalFormID(armor);
    }

    std::string BuildKeywordCommentLine(
        const std::string_view keyword,
        const std::string_view keywordDescription)
    {
        return "; " + std::string(keyword) + ": " +
               std::string(keywordDescription);
    }

    std::string BuildRuleLine(
        const std::string_view keyword,
        const std::string& filter)
    {
        return "Keyword = " + std::string(keyword) + "|Armor|" + filter;
    }

    std::string BuildEntryBlock(
        RE::TESObjectARMO* armor,
        const std::string& displayName,
        const std::string_view keyword,
        const std::string_view keywordDescription,
        const std::string& ruleLine)
    {
        std::ostringstream output;
        output << BuildCommentLine(armor, displayName) << '\n';
        output << BuildKeywordCommentLine(keyword, keywordDescription) << '\n';
        output << ruleLine;
        return output.str();
    }

    bool IsCommentLine(const std::string& line)
    {
        return Normalize(line).starts_with(';');
    }

    std::vector<std::string> SplitLines(const std::string& content)
    {
        std::vector<std::string> lines;
        std::istringstream input(content);
        std::string line;

        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            lines.push_back(std::move(line));
        }

        return lines;
    }

    std::string JoinLines(const std::vector<std::string>& lines)
    {
        std::ostringstream output;

        for (std::size_t index = 0; index < lines.size(); ++index) {
            if (index > 0) {
                output << '\n';
            }

            output << lines[index];
        }

        if (!lines.empty()) {
            output << '\n';
        }

        return output.str();
    }

    bool IsArmorRuleForFilter(
        const std::string& line,
        const std::vector<std::string>& filters)
    {
        const auto normalizedLine = Normalize(line);
        if (!normalizedLine.starts_with("keyword = ")) {
            return false;
        }

        for (const auto& filter : filters) {
            if (filter.empty()) {
                continue;
            }

            const auto normalizedNeedle = "|armor|" + Normalize(filter);
            if (normalizedLine.ends_with(normalizedNeedle)) {
                return true;
            }
        }

        return false;
    }

    std::vector<std::string> BuildEntryLines(
        RE::TESObjectARMO* armor,
        const std::string& displayName,
        const std::string_view keyword,
        const std::string_view keywordDescription,
        const std::string& ruleLine)
    {
        return {
            BuildCommentLine(armor, displayName),
            BuildKeywordCommentLine(keyword, keywordDescription),
            ruleLine
        };
    }
}

namespace NPCEquipmentViewer
{
    KidWriter::WriteResult KidWriter::AddArmor(
        RE::TESObjectARMO* armor,
        const std::string& displayName,
        const std::string_view keyword,
        const std::string_view keywordDescription)
    {
        WriteResult result;
        result.path = GetOutputPath();

        if (armor == nullptr || keyword.empty() || keywordDescription.empty()) {
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

        result.line = BuildRuleLine(keyword, selectedFilter);
        std::vector<std::string> itemFilters{ selectedFilter };

        if (!exactFilter.empty() && exactFilter != selectedFilter) {
            itemFilters.push_back(exactFilter);
        }
        if (!fallbackFilter.empty() && fallbackFilter != selectedFilter) {
            itemFilters.push_back(fallbackFilter);
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

            auto lines = SplitLines(existingContent);
            for (std::size_t index = 0; index < lines.size(); ++index) {
                if (!IsArmorRuleForFilter(lines[index], itemFilters)) {
                    continue;
                }

                const auto replacement = BuildEntryLines(
                    armor,
                    displayName,
                    keyword,
                    keywordDescription,
                    result.line);

                if (Normalize(lines[index]) == Normalize(result.line)) {
                    if (index >= replacement.size() - 1 &&
                        Normalize(lines[index - 1]) ==
                            Normalize(replacement[replacement.size() - 2])) {
                        result.result = Result::kDuplicate;
                        return result;
                    }
                }

                auto eraseBegin = index;
                while (eraseBegin > 0 && IsCommentLine(lines[eraseBegin - 1])) {
                    --eraseBegin;
                }

                lines.erase(lines.begin() + static_cast<std::ptrdiff_t>(eraseBegin),
                    lines.begin() + static_cast<std::ptrdiff_t>(index + 1));
                lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(eraseBegin),
                    replacement.begin(),
                    replacement.end());

                std::ofstream replacementOutput(
                    result.path,
                    std::ios::binary | std::ios::trunc);
                if (!replacementOutput.is_open()) {
                    result.result = Result::kFileError;
                    return result;
                }

                replacementOutput << JoinLines(lines);
                replacementOutput.flush();

                result.result = replacementOutput.good()
                    ? Result::kUpdated
                    : Result::kFileError;
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

        output << BuildEntryBlock(
            armor,
            displayName,
            keyword,
            keywordDescription,
            result.line) << '\n';
        output.flush();

        result.result = output.good() ? Result::kAdded : Result::kFileError;
        return result;
    }

    std::filesystem::path KidWriter::GetOutputPath()
    {
        return GetDataDirectory() / kOutputFileName;
    }
}
