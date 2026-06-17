#include "PCH.h"
#include "UIExtensionsMenu.h"

namespace
{
    using ResultHandler = std::function<void(RE::BSScript::Variable)>;

    constexpr std::int32_t kMenuError = -2;
    constexpr std::string_view kNativeScriptName = "NPCEquipmentViewerNative";
    constexpr std::string_view kMenuScriptName = "NPCEquipmentViewerMenu";
    constexpr std::string_view kCustomMenuName = "CustomMenu";
    constexpr float kListWidth = 384.0F;
    constexpr float kStageWidth = 1280.0F;

    struct MenuSession
    {
        std::vector<std::string> entries;
        NPCEquipmentViewer::UIExtensionsMenu::SelectionCallback callback;
        bool active{ false };
        bool resizePending{ false };
    };

    std::mutex g_sessionMutex;
    MenuSession g_session;

    class VmCallback final : public RE::BSScript::IStackCallbackFunctor
    {
    public:
        explicit VmCallback(ResultHandler handler) :
            handler_(std::move(handler))
        {}

        void operator()(RE::BSScript::Variable result) override
        {
            if (handler_) {
                handler_(std::move(result));
            }
        }

        bool CanSave() const override
        {
            return false;
        }

        void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) override
        {}

    private:
        ResultHandler handler_;
    };

    void ResizeCurrentListMenu()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui == nullptr) {
            return;
        }

        const auto menu = ui->GetMenu(kCustomMenuName.data());
        if (!menu || !menu->uiMovie) {
            return;
        }

        const RE::GFxValue widthValue{ kListWidth };
        const RE::GFxValue positionValue{ (kStageWidth - kListWidth) / 2.0F };

        menu->uiMovie->SetVariable(
            "_root.listMenu.itemView.background._width",
            widthValue);
        menu->uiMovie->SetVariable(
            "_root.listMenu.itemList.background._width",
            widthValue);
        menu->uiMovie->SetVariable(
            "_root.listMenu.itemList.scrollbar._x",
            widthValue);
        menu->uiMovie->SetVariable(
            "_root.listMenu._x",
            positionValue);
    }

    class MenuResizeListener final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuResizeListener& GetSingleton()
        {
            static MenuResizeListener singleton;
            return singleton;
        }

        void Register()
        {
            if (registered_) {
                return;
            }

            auto* ui = RE::UI::GetSingleton();
            if (ui == nullptr) {
                return;
            }

            ui->AddEventSink<RE::MenuOpenCloseEvent>(this);
            registered_ = true;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent* event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (event == nullptr ||
                !event->opening ||
                event->menuName != kCustomMenuName) {
                return RE::BSEventNotifyControl::kContinue;
            }

            bool shouldResize = false;

            {
                std::scoped_lock lock(g_sessionMutex);

                if (g_session.active && g_session.resizePending) {
                    g_session.resizePending = false;
                    shouldResize = true;
                }
            }

            if (shouldResize) {
                if (const auto* taskInterface = SKSE::GetTaskInterface(); taskInterface != nullptr) {
                    taskInterface->AddTask([]() {
                        ResizeCurrentListMenu();
                    });
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }

    private:
        bool registered_{ false };
    };

    RE::BSScript::IVirtualMachine* GetVirtualMachine()
    {
        auto* skyrimVM = RE::SkyrimVM::GetSingleton();
        if (skyrimVM == nullptr || !skyrimVM->impl) {
            return nullptr;
        }

        return skyrimVM->impl.get();
    }

    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> MakeCallback(ResultHandler handler)
    {
        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
        callback.reset(new VmCallback(std::move(handler)));
        return callback;
    }

    void ClearSession()
    {
        std::scoped_lock lock(g_sessionMutex);
        g_session.entries.clear();
        g_session.callback = nullptr;
        g_session.active = false;
        g_session.resizePending = false;
    }

    void CompleteSession(const std::int32_t selectedIndex)
    {
        NPCEquipmentViewer::UIExtensionsMenu::SelectionCallback callback;

        {
            std::scoped_lock lock(g_sessionMutex);

            if (!g_session.active) {
                return;
            }

            callback = std::move(g_session.callback);
            g_session.entries.clear();
            g_session.active = false;
            g_session.resizePending = false;
        }

        if (callback) {
            callback(selectedIndex);
        }
    }

    std::int32_t GetEntryCount(RE::StaticFunctionTag*)
    {
        std::scoped_lock lock(g_sessionMutex);

        if (!g_session.active) {
            return 0;
        }

        return static_cast<std::int32_t>(g_session.entries.size());
    }

    RE::BSFixedString GetEntryText(
        RE::StaticFunctionTag*,
        const std::int32_t index)
    {
        std::scoped_lock lock(g_sessionMutex);

        if (!g_session.active ||
            index < 0 ||
            static_cast<std::size_t>(index) >= g_session.entries.size()) {
            return {};
        }

        return RE::BSFixedString(g_session.entries[static_cast<std::size_t>(index)]);
    }
}

namespace NPCEquipmentViewer
{
    bool UIExtensionsMenu::IsAvailable()
    {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        return dataHandler != nullptr &&
               dataHandler->LookupLoadedModByName("UIExtensions.esp") != nullptr &&
               GetVirtualMachine() != nullptr;
    }

    void UIExtensionsMenu::RegisterMenuEvents()
    {
        MenuResizeListener::GetSingleton().Register();
    }

    bool UIExtensionsMenu::RegisterPapyrusFunctions(
        RE::BSScript::IVirtualMachine* virtualMachine)
    {
        if (virtualMachine == nullptr) {
            return false;
        }

        virtualMachine->RegisterFunction(
            "GetEntryCount",
            kNativeScriptName.data(),
            GetEntryCount);

        virtualMachine->RegisterFunction(
            "GetEntryText",
            kNativeScriptName.data(),
            GetEntryText);

        return true;
    }

    bool UIExtensionsMenu::Show(
        const std::vector<std::string>& entries,
        SelectionCallback callback)
    {
        if (entries.empty() || !callback || !IsAvailable()) {
            return false;
        }

        auto* virtualMachine = GetVirtualMachine();
        if (virtualMachine == nullptr) {
            return false;
        }

        {
            std::scoped_lock lock(g_sessionMutex);

            if (g_session.active) {
                return false;
            }

            g_session.entries = entries;
            g_session.callback = std::move(callback);
            g_session.active = true;
            g_session.resizePending = true;
        }

        auto vmCallback = MakeCallback([](RE::BSScript::Variable result) {
            if (!result.IsInt()) {
                CompleteSession(kMenuError);
                return;
            }

            CompleteSession(result.GetSInt());
        });

        if (!virtualMachine->DispatchStaticCall(
                kMenuScriptName.data(),
                "ShowEquipmentMenu",
                RE::MakeFunctionArguments(),
                vmCallback)) {
            ClearSession();
            return false;
        }

        return true;
    }
}
