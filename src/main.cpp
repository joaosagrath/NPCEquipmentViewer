#include "PCH.h"
#include "EventHandler.h"
#include "Settings.h"
#include "UIExtensionsMenu.h"

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    NPCEquipmentViewer::Settings::GetSingleton().Load();

    if (auto* papyrusInterface = SKSE::GetPapyrusInterface(); papyrusInterface != nullptr) {
        papyrusInterface->Register(
            NPCEquipmentViewer::UIExtensionsMenu::RegisterPapyrusFunctions);
    }

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message != nullptr && message->type == SKSE::MessagingInterface::kDataLoaded) {
            NPCEquipmentViewer::UIExtensionsMenu::RegisterMenuEvents();
            NPCEquipmentViewer::EventHandler::Register();
        }
    });

    return true;
}
