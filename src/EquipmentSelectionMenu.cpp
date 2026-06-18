#include "PCH.h"
#include "EquipmentSelectionMenu.h"

namespace
{
    constexpr std::string_view kMenuName = "NPCEquipmentViewerMenu";
    constexpr std::string_view kSetDataMethod =
        "_root.NPCEquipmentViewerMenu.SetData";
    constexpr std::size_t kVisibleRows = 10;

    struct MenuRequest
    {
        std::string title;
        std::vector<std::string> entries;
        NPCEquipmentViewer::EquipmentSelectionMenu::SelectionCallback callback;
    };

    std::mutex g_requestMutex;
    std::optional<MenuRequest> g_pendingRequest;

    class ScaleformEquipmentMenu final : public RE::IMenu
    {
    public:
        ScaleformEquipmentMenu()
        {
            depthPriority = 10;
            inputContext = Context::kMenuMode;
            menuName = kMenuName.data();

            menuFlags.set(
                Flag::kPausesGame,
                Flag::kModal,
                Flag::kUsesMenuContext,
                Flag::kDisablePauseMenu);

            auto* scaleformManager = RE::BSScaleformManager::GetSingleton();
            if (scaleformManager == nullptr) {
                SKSE::log::error(
                    "[MenuDiagnostic] BSScaleformManager singleton is unavailable");
                return;
            }

            if (!scaleformManager->LoadMovie(
                    this,
                    uiMovie,
                    kMenuName.data())) {
                SKSE::log::error(
                    "[MenuDiagnostic] Could not load Interface/{}.swf",
                    kMenuName);
                return;
            }

            SKSE::log::info(
                "[MenuDiagnostic] Loaded Interface/{}.swf successfully",
                kMenuName);
        }

        static RE::IMenu* Create()
        {
            return new ScaleformEquipmentMenu();
        }

        RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& message) override
        {
            switch (message.type.get()) {
            case RE::UI_MESSAGE_TYPE::kShow:
                SKSE::log::info(
                    "[MenuDiagnostic] Received menu show message");
                OnShow();
                break;

            case RE::UI_MESSAGE_TYPE::kHide:
                SKSE::log::info(
                    "[MenuDiagnostic] Received menu hide message");
                request_.reset();
                break;

            case RE::UI_MESSAGE_TYPE::kUserEvent:
                if (message.data != nullptr) {
                    const auto* eventData =
                        reinterpret_cast<RE::BSUIMessageData*>(message.data);
                    ProcessUserEvent(eventData->fixedStr);
                }
                break;

            default:
                break;
            }

            return RE::UI_MESSAGE_RESULTS::kHandled;
        }

    private:
        void OnShow()
        {
            {
                std::scoped_lock lock(g_requestMutex);

                if (g_pendingRequest.has_value()) {
                    request_ = std::move(g_pendingRequest);
                    g_pendingRequest.reset();
                }
            }

            if (!request_.has_value()) {
                SKSE::log::error(
                    "[MenuDiagnostic] Menu opened without a pending request");
                Close();
                return;
            }

            if (request_->entries.empty()) {
                SKSE::log::error(
                    "[MenuDiagnostic] Menu request contains no equipment entries");
                Close();
                return;
            }

            if (!uiMovie) {
                SKSE::log::error(
                    "[MenuDiagnostic] Menu request is valid, but uiMovie is null");
                Close();
                return;
            }

            selectedIndex_ = 0;
            firstVisibleIndex_ = 0;

            SKSE::log::info(
                "[MenuDiagnostic] Opening menu: title='{}', entries={}",
                request_->title,
                request_->entries.size());

            for (std::size_t index = 0;
                 index < request_->entries.size();
                 ++index) {
                SKSE::log::info(
                    "[MenuDiagnostic] Native entry[{}]='{}'",
                    index,
                    request_->entries[index]);
            }

            Refresh();
        }

        void ProcessUserEvent(const RE::BSFixedString& eventName)
        {
            if (!request_.has_value() || request_->entries.empty()) {
                SKSE::log::warn(
                    "[MenuDiagnostic] Ignored input because the menu request is unavailable");
                return;
            }

            const auto* userEvents = RE::UserEvents::GetSingleton();
            if (userEvents == nullptr) {
                SKSE::log::error(
                    "[MenuDiagnostic] UserEvents singleton is unavailable");
                return;
            }

            if (eventName == userEvents->up ||
                eventName == userEvents->prevFocus ||
                eventName == userEvents->pickPrevious) {
                MoveSelection(-1);
                return;
            }

            if (eventName == userEvents->down ||
                eventName == userEvents->nextFocus ||
                eventName == userEvents->pickNext) {
                MoveSelection(1);
                return;
            }

            if (eventName == userEvents->pageUp ||
                eventName == userEvents->prevPage ||
                eventName == userEvents->left) {
                MovePage(-1);
                return;
            }

            if (eventName == userEvents->pageDown ||
                eventName == userEvents->nextPage ||
                eventName == userEvents->right) {
                MovePage(1);
                return;
            }

            if (eventName == userEvents->accept ||
                eventName == userEvents->activate) {
                ConfirmSelection();
                return;
            }

            if (eventName == userEvents->cancel) {
                Close();
            }
        }

        void MoveSelection(const int direction)
        {
            const auto itemCount = request_->entries.size();
            if (itemCount == 0) {
                return;
            }

            if (direction < 0) {
                selectedIndex_ =
                    selectedIndex_ == 0 ? itemCount - 1 : selectedIndex_ - 1;
            } else {
                selectedIndex_ = (selectedIndex_ + 1) % itemCount;
            }

            EnsureSelectionIsVisible();
            Refresh();
        }

        void MovePage(const int direction)
        {
            const auto itemCount = request_->entries.size();
            if (itemCount == 0) {
                return;
            }

            if (direction < 0) {
                selectedIndex_ =
                    selectedIndex_ > kVisibleRows
                        ? selectedIndex_ - kVisibleRows
                        : 0;
            } else {
                selectedIndex_ = std::min(
                    selectedIndex_ + kVisibleRows,
                    itemCount - 1);
            }

            EnsureSelectionIsVisible();
            Refresh();
        }

        void EnsureSelectionIsVisible()
        {
            if (selectedIndex_ < firstVisibleIndex_) {
                firstVisibleIndex_ = selectedIndex_;
                return;
            }

            if (selectedIndex_ >= firstVisibleIndex_ + kVisibleRows) {
                firstVisibleIndex_ =
                    selectedIndex_ - kVisibleRows + 1;
            }
        }

        void Refresh()
        {
            if (!request_.has_value()) {
                SKSE::log::error(
                    "[MenuDiagnostic] Refresh failed: request is unavailable");
                return;
            }

            if (!uiMovie) {
                SKSE::log::error(
                    "[MenuDiagnostic] Refresh failed: uiMovie is null");
                return;
            }

            const auto lastVisibleIndex = std::min(
                firstVisibleIndex_ + kVisibleRows,
                request_->entries.size());
            const auto expectedRows =
                lastVisibleIndex - firstVisibleIndex_;
            const auto selectedRow =
                selectedIndex_ - firstVisibleIndex_;

            std::array<std::string, kVisibleRows> visibleRows;

            for (auto absoluteIndex = firstVisibleIndex_;
                 absoluteIndex < lastVisibleIndex;
                 ++absoluteIndex) {
                const auto relativeIndex =
                    absoluteIndex - firstVisibleIndex_;
                visibleRows[relativeIndex] =
                    request_->entries[absoluteIndex];

                std::replace(
                    visibleRows[relativeIndex].begin(),
                    visibleRows[relativeIndex].end(),
                    '\r',
                    ' ');
                std::replace(
                    visibleRows[relativeIndex].begin(),
                    visibleRows[relativeIndex].end(),
                    '\n',
                    ' ');

                SKSE::log::info(
                    "[MenuDiagnostic] Visible row[{}], absolute entry[{}]='{}'",
                    relativeIndex,
                    absoluteIndex,
                    visibleRows[relativeIndex]);
            }

            std::array<RE::GFxValue, 14> arguments;
            arguments[0].SetString(request_->title.c_str());
            arguments[1].SetNumber(static_cast<double>(selectedRow));
            arguments[2].SetNumber(
                static_cast<double>(firstVisibleIndex_));
            arguments[3].SetNumber(
                static_cast<double>(request_->entries.size()));

            for (std::size_t rowIndex = 0;
                 rowIndex < kVisibleRows;
                 ++rowIndex) {
                arguments[4 + rowIndex].SetString(
                    visibleRows[rowIndex].c_str());
            }

            SKSE::log::info(
                "[MenuDiagnostic] Invoking SetData with explicit row fields: selectedIndex={}, selectedRow={}, firstVisibleIndex={}, expectedRows={}, totalEntries={}",
                selectedIndex_,
                selectedRow,
                firstVisibleIndex_,
                expectedRows,
                request_->entries.size());

            RE::GFxValue result;
            const bool invoked = uiMovie->Invoke(
                kSetDataMethod.data(),
                &result,
                arguments.data(),
                static_cast<std::uint32_t>(arguments.size()));

            if (!invoked) {
                SKSE::log::error(
                    "[MenuDiagnostic] SetData invocation failed. The DLL and SWF may be from different versions");
                return;
            }

            const double renderedRows =
                result.IsNumber() ? result.GetNumber() : -1.0;

            SKSE::log::info(
                "[MenuDiagnostic] SetData invoked successfully; returnedType={}, renderedRows={}, expectedRows={}",
                static_cast<std::uint32_t>(result.GetType()),
                renderedRows,
                expectedRows);

            if (renderedRows <= -1000.0 && renderedRows > -2000.0) {
                SKSE::log::error(
                    "[MenuDiagnostic] ActionScript could not find row background {}",
                    static_cast<int>(-1000.0 - renderedRows));
            } else if (renderedRows <= -2000.0 && renderedRows > -3000.0) {
                SKSE::log::error(
                    "[MenuDiagnostic] ActionScript could not find row text field {}",
                    static_cast<int>(-2000.0 - renderedRows));
            } else if (renderedRows == -9000.0) {
                SKSE::log::error(
                    "[MenuDiagnostic] ActionScript found the row components, but all ten row strings were empty");
            } else if (renderedRows != static_cast<double>(expectedRows)) {
                SKSE::log::error(
                    "[MenuDiagnostic] Row rendering mismatch: renderedRows={}, expectedRows={}",
                    renderedRows,
                    expectedRows);
            }
        }

        void ConfirmSelection()
        {
            if (!request_.has_value() ||
                selectedIndex_ >= request_->entries.size()) {
                SKSE::log::error(
                    "[MenuDiagnostic] ConfirmSelection failed: invalid request or selected index");
                return;
            }

            SKSE::log::info(
                "[MenuDiagnostic] Confirmed native entry[{}]='{}'",
                selectedIndex_,
                request_->entries[selectedIndex_]);

            auto callback = std::move(request_->callback);
            const auto selectedIndex = selectedIndex_;
            request_.reset();

            Close();

            if (!callback) {
                SKSE::log::error(
                    "[MenuDiagnostic] ConfirmSelection failed: callback is empty");
                return;
            }

            if (const auto* taskInterface = SKSE::GetTaskInterface();
                taskInterface != nullptr) {
                taskInterface->AddTask(
                    [callback = std::move(callback), selectedIndex]() mutable {
                        callback(selectedIndex);
                    });
            } else {
                SKSE::log::warn(
                    "[MenuDiagnostic] TaskInterface unavailable; executing selection callback immediately");
                callback(selectedIndex);
            }
        }

        static void Close()
        {
            if (auto* messageQueue = RE::UIMessageQueue::GetSingleton();
                messageQueue != nullptr) {
                messageQueue->AddMessage(
                    kMenuName.data(),
                    RE::UI_MESSAGE_TYPE::kHide,
                    nullptr);
            } else {
                SKSE::log::error(
                    "[MenuDiagnostic] Could not close menu: UIMessageQueue singleton is unavailable");
            }
        }

        std::optional<MenuRequest> request_;
        std::size_t selectedIndex_{ 0 };
        std::size_t firstVisibleIndex_{ 0 };
    };
}

namespace NPCEquipmentViewer
{
    void EquipmentSelectionMenu::Register()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui == nullptr) {
            SKSE::log::error(
                "[MenuDiagnostic] Could not register {}: UI singleton is unavailable",
                kMenuName);
            return;
        }

        ui->Register(kMenuName, ScaleformEquipmentMenu::Create);
        SKSE::log::info(
            "[MenuDiagnostic] Registered custom menu {}",
            kMenuName);
    }

    bool EquipmentSelectionMenu::Show(
        std::string title,
        std::vector<std::string> entries,
        SelectionCallback callback)
    {
        if (entries.empty()) {
            SKSE::log::error(
                "[MenuDiagnostic] Show rejected: entries vector is empty");
            return false;
        }

        if (!callback) {
            SKSE::log::error(
                "[MenuDiagnostic] Show rejected: selection callback is empty");
            return false;
        }

        auto* ui = RE::UI::GetSingleton();
        if (ui == nullptr) {
            SKSE::log::error(
                "[MenuDiagnostic] Show rejected: UI singleton is unavailable");
            return false;
        }

        auto* messageQueue = RE::UIMessageQueue::GetSingleton();
        if (messageQueue == nullptr) {
            SKSE::log::error(
                "[MenuDiagnostic] Show rejected: UIMessageQueue singleton is unavailable");
            return false;
        }

        if (ui->IsMenuOpen(kMenuName)) {
            SKSE::log::warn(
                "[MenuDiagnostic] Show rejected: {} is already open",
                kMenuName);
            return false;
        }

        {
            std::scoped_lock lock(g_requestMutex);

            if (g_pendingRequest.has_value()) {
                SKSE::log::warn(
                    "[MenuDiagnostic] Show rejected: another menu request is pending");
                return false;
            }

            SKSE::log::info(
                "[MenuDiagnostic] Queueing menu request: title='{}', entries={}",
                title,
                entries.size());

            g_pendingRequest = MenuRequest{
                std::move(title),
                std::move(entries),
                std::move(callback)
            };
        }

        messageQueue->AddMessage(
            kMenuName.data(),
            RE::UI_MESSAGE_TYPE::kShow,
            nullptr);
        return true;
    }
}
