#include "PCH.h"
#include "MessageBox.h"

namespace
{
    class MessageBoxCallback final : public RE::IMessageBoxCallback
    {
    public:
        explicit MessageBoxCallback(NPCEquipmentViewer::MessageBox::Callback callback) :
            callback_(std::move(callback))
        {}

        void Run(Message message) override
        {
            if (callback_) {
                callback_(static_cast<std::uint32_t>(message));
            }
        }

    private:
        NPCEquipmentViewer::MessageBox::Callback callback_;
    };
}

namespace NPCEquipmentViewer
{
    bool MessageBox::Show(
        const std::string& body,
        const std::vector<std::string>& buttons,
        Callback callback)
    {
        if (buttons.empty()) {
            return false;
        }

        auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
        auto* interfaceStrings = RE::InterfaceStrings::GetSingleton();

        if (factoryManager == nullptr || interfaceStrings == nullptr) {
            return false;
        }

        auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(interfaceStrings->messageBoxData);
        if (factory == nullptr) {
            return false;
        }

        auto* messageBox = factory->Create();
        if (messageBox == nullptr) {
            return false;
        }

        messageBox->callback = RE::make_smart<MessageBoxCallback>(std::move(callback));
        messageBox->bodyText = body.c_str();

        for (const auto& button : buttons) {
            messageBox->buttonText.push_back(button.c_str());
        }

        messageBox->QueueMessage();
        return true;
    }
}
