#include "PCH.h"
#include "Settings.h"

namespace
{
    std::string Trim(std::string value)
    {
        const auto notSpace = [](const unsigned char character) {
            return std::isspace(character) == 0;
        };

        value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
        value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
        return value;
    }

    std::string ToLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    bool ParseBoolean(const std::string& value, const bool fallback)
    {
        const auto normalized = ToLower(Trim(value));

        if (normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "sim" || normalized == "on") {
            return true;
        }

        if (normalized == "0" || normalized == "false" || normalized == "no" || normalized == "nao" || normalized == "off") {
            return false;
        }

        return fallback;
    }

    std::filesystem::path GetPluginDirectory()
    {
        static const int moduleAnchor = 0;

        HMODULE moduleHandle = nullptr;
        const auto flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

        if (GetModuleHandleExW(flags, reinterpret_cast<LPCWSTR>(&moduleAnchor), &moduleHandle) != 0) {
            std::array<wchar_t, 32768> modulePath{};
            const auto length = GetModuleFileNameW(moduleHandle, modulePath.data(), static_cast<DWORD>(modulePath.size()));

            if (length > 0 && length < modulePath.size()) {
                return std::filesystem::path(modulePath.data()).parent_path();
            }
        }

        return std::filesystem::current_path() / "Data" / "SKSE" / "Plugins";
    }
}

namespace NPCEquipmentViewer
{
    Settings& Settings::GetSingleton()
    {
        static Settings singleton;
        return singleton;
    }

    void Settings::Load()
    {
        keyCode_ = 0x25;
        showFormID_ = true;
        showSlots_ = true;
        showItemType_ = true;

        const auto iniPath = GetPluginDirectory() / "NPCEquipmentViewer.ini";
        std::ifstream input(iniPath);

        if (!input.is_open()) {
            return;
        }

        std::string section;
        std::string line;

        while (std::getline(input, line)) {
            line = Trim(line);

            if (line.empty() || line.front() == ';' || line.front() == '#') {
                continue;
            }

            if (line.front() == '[' && line.back() == ']') {
                section = ToLower(Trim(line.substr(1, line.size() - 2)));
                continue;
            }

            if (section != "general") {
                continue;
            }

            const auto separator = line.find('=');
            if (separator == std::string::npos) {
                continue;
            }

            const auto key = ToLower(Trim(line.substr(0, separator)));
            const auto value = Trim(line.substr(separator + 1));

            if (key == "keycode") {
                try {
                    const auto parsedValue = std::stoul(value, nullptr, 0);
                    if (parsedValue <= 0xFF) {
                        keyCode_ = static_cast<std::uint32_t>(parsedValue);
                    }
                } catch (...) {
                    keyCode_ = 0x25;
                }
            } else if (key == "showformid") {
                showFormID_ = ParseBoolean(value, showFormID_);
            } else if (key == "showslots") {
                showSlots_ = ParseBoolean(value, showSlots_);
            } else if (key == "showitemtype") {
                showItemType_ = ParseBoolean(value, showItemType_);
            }
        }
    }

    std::uint32_t Settings::GetKeyCode() const noexcept
    {
        return keyCode_;
    }

    bool Settings::ShowFormID() const noexcept
    {
        return showFormID_;
    }

    bool Settings::ShowSlots() const noexcept
    {
        return showSlots_;
    }

    bool Settings::ShowItemType() const noexcept
    {
        return showItemType_;
    }
}
