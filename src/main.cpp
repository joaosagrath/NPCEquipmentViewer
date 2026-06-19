#include "PCH.h"
#include "EquipmentSelectionMenu.h"
#include "EventHandler.h"
#include "Settings.h"

#include <spdlog/sinks/basic_file_sink.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace
{
    constexpr std::string_view kLogFileName =
        "Equipment Viewer and KID Writer.log";

    std::filesystem::path GetPluginDirectory()
    {
        std::array<wchar_t, MAX_PATH> modulePath{};
        const auto length = GetModuleFileNameW(
            reinterpret_cast<HMODULE>(&__ImageBase),
            modulePath.data(),
            static_cast<DWORD>(modulePath.size()));

        if (length == 0 || length >= modulePath.size()) {
            return {};
        }

        return std::filesystem::path(modulePath.data()).parent_path();
    }

    bool TryInitializeLogging(const std::filesystem::path& logPath)
    {
        try {
            if (const auto parentPath = logPath.parent_path();
                !parentPath.empty()) {
                std::filesystem::create_directories(parentPath);
            }

            auto sink = std::make_shared<
                spdlog::sinks::basic_file_sink_mt>(
                    logPath.string(),
                    true);
            auto logger = std::make_shared<spdlog::logger>(
                "NPCEquipmentViewer",
                std::move(sink));

            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
            logger->set_level(spdlog::level::info);
            logger->flush_on(spdlog::level::info);
            spdlog::set_default_logger(std::move(logger));

            SKSE::log::info(
                "[MenuDiagnostic] Logging initialized at '{}'",
                logPath.string());
            return true;
        } catch (const std::exception& exception) {
            const auto message = std::string(
                "Equipment Viewer and KID Writer could not initialize log at '") +
                logPath.string() +
                "': " +
                exception.what() +
                "\n";
            OutputDebugStringA(message.c_str());
            return false;
        }
    }

    void InitializeLogging()
    {
        std::vector<std::filesystem::path> candidatePaths;

        if (const auto logDirectory = SKSE::log::log_directory();
            logDirectory.has_value()) {
            candidatePaths.push_back(
                *logDirectory / kLogFileName);
        }

        if (const auto pluginDirectory = GetPluginDirectory();
            !pluginDirectory.empty()) {
            candidatePaths.push_back(
                pluginDirectory / kLogFileName);
        }

        try {
            candidatePaths.push_back(
                std::filesystem::current_path() /
                kLogFileName);
        } catch (const std::exception&) {
        }

        for (const auto& candidatePath : candidatePaths) {
            if (TryInitializeLogging(candidatePath)) {
                return;
            }
        }

        OutputDebugStringA(
            "Equipment Viewer and KID Writer could not create a log file in any candidate directory.\n");
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    InitializeLogging();

    SKSE::log::info(
        "[MenuDiagnostic] Equipment Viewer and KID Writer plugin loading");

    NPCEquipmentViewer::Settings::GetSingleton().Load();

    SKSE::GetMessagingInterface()->RegisterListener(
        [](SKSE::MessagingInterface::Message* message) {
            if (message != nullptr &&
                message->type ==
                    SKSE::MessagingInterface::kDataLoaded) {
                SKSE::log::info(
                    "[MenuDiagnostic] Received kDataLoaded message");
                NPCEquipmentViewer::EquipmentSelectionMenu::Register();
                NPCEquipmentViewer::EventHandler::Register();
            }
        });

    return true;
}
