#include "PCH.h"
#include "EventHandler.h"
#include "Settings.h"

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    NPCEquipmentViewer::Settings::GetSingleton().Load();

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message != nullptr && message->type == SKSE::MessagingInterface::kDataLoaded) {
            NPCEquipmentViewer::EventHandler::Register();
        }
    });

    return true;
}
