#include "PCH.h"
#include "UIExtensionsMenu.h"

namespace
{
    using ScriptObject = RE::BSTSmartPointer<RE::BSScript::Object>;
    using ResultHandler = std::function<void(RE::BSScript::Variable)>;

    constexpr std::int32_t kMenuError = -2;

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

        void SetObject(const ScriptObject&) override
        {}

    private:
        ResultHandler handler_;
    };

    struct MenuRequest
    {
        ScriptObject menu;
        std::vector<std::string> entries;
        NPCEquipmentViewer::UIExtensionsMenu::SelectionCallback callback;
        std::size_t nextEntry{ 0 };
        bool completed{ false };
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

    void CompleteRequest(
        const std::shared_ptr<MenuRequest>& request,
        const std::int32_t selectedIndex)
    {
        if (!request || request->completed) {
            return;
        }

        request->completed = true;

        if (request->callback) {
            request->callback(selectedIndex);
        }
    }

    bool DispatchMethod(
        const ScriptObject& object,
        const char* functionName,
        RE::BSScript::IFunctionArguments* arguments,
        ResultHandler handler)
    {
        auto* vm = GetVirtualMachine();
        if (vm == nullptr || !object) {
            delete arguments;
            return false;
        }

        auto objectCopy = object;
        auto callback = MakeCallback(std::move(handler));

        if (!vm->DispatchMethodCall1(objectCopy, functionName, arguments, callback)) {
            delete arguments;
            return false;
        }

        return true;
    }

    void ReadSelectedIndex(const std::shared_ptr<MenuRequest>& request)
    {
        const bool dispatched = DispatchMethod(
            request->menu,
            "GetResultInt",
            RE::MakeFunctionArguments(),
            [request](RE::BSScript::Variable result) {
                if (!result.IsInt()) {
                    CompleteRequest(request, kMenuError);
                    return;
                }

                CompleteRequest(request, result.GetSInt());
            });

        if (!dispatched) {
            CompleteRequest(request, kMenuError);
        }
    }

    void OpenListMenu(const std::shared_ptr<MenuRequest>& request)
    {
        const bool dispatched = DispatchMethod(
            request->menu,
            "OpenMenu",
            RE::MakeFunctionArguments(),
            [request](RE::BSScript::Variable) {
                ReadSelectedIndex(request);
            });

        if (!dispatched) {
            CompleteRequest(request, kMenuError);
        }
    }

    void AddNextEntry(const std::shared_ptr<MenuRequest>& request)
    {
        if (request->nextEntry >= request->entries.size()) {
            OpenListMenu(request);
            return;
        }

        const auto currentIndex = request->nextEntry;
        const auto callbackValue = static_cast<std::int32_t>(currentIndex);

        const bool dispatched = DispatchMethod(
            request->menu,
            "AddEntryItem",
            RE::MakeFunctionArguments(
                request->entries[currentIndex],
                static_cast<std::int32_t>(-1),
                callbackValue,
                false),
            [request](RE::BSScript::Variable) {
                ++request->nextEntry;
                AddNextEntry(request);
            });

        if (!dispatched) {
            CompleteRequest(request, kMenuError);
        }
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

    bool UIExtensionsMenu::Show(
        const std::vector<std::string>& entries,
        SelectionCallback callback)
    {
        if (entries.empty() || !IsAvailable()) {
            return false;
        }

        auto* vm = GetVirtualMachine();
        if (vm == nullptr) {
            return false;
        }

        auto request = std::make_shared<MenuRequest>();
        request->entries = entries;
        request->callback = std::move(callback);

        auto vmCallback = MakeCallback([request](RE::BSScript::Variable result) {
            if (!result.IsObject()) {
                CompleteRequest(request, kMenuError);
                return;
            }

            request->menu = result.GetObject();
            if (!request->menu || !request->menu->IsValid()) {
                CompleteRequest(request, kMenuError);
                return;
            }

            AddNextEntry(request);
        });

        auto* arguments = RE::MakeFunctionArguments(std::string("UIListMenu"), true);

        if (!vm->DispatchStaticCall(
                "UIExtensions",
                "GetMenu",
                arguments,
                vmCallback)) {
            delete arguments;
            return false;
        }

        return true;
    }
}
