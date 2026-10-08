#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"

namespace SEASON3B
{
    // The dialog of Warden Eldrin (Illusion of Noria, GameLogic/Events/IllusionOfNoria.h): the quest of the whistle, the
    // entry, the daily quest, the reset of the harmony option of the equipped weapons and, after the quest, the shop for
    // Illusion Shards; every purchase asks for a confirmation.
    class CIllusionWardenMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CIllusionWardenMsgBox() = default;
        ~CIllusionWardenMsgBox() override { Release(); }

        bool Create(float fPriority = 3.f);
        void Release() override;

        bool Update() override;
        bool Render() override;

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CloseBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        enum class Page
        {
            Main,
            Shop,
            Confirm,
        };

        // what a button does
        enum class Command
        {
            Action,  // sends the action to the server and closes
            Shop,    // opens the shop page
            Back,    // back to the main page
            Buy,     // asks for the confirmation of the purchase
            Confirm, // sends the purchase
        };

        struct Line
        {
            std::wstring Text;
            DWORD Color;
            bool Bold;
        };

        struct Button
        {
            std::wstring Text;
            Command Kind;
            std::uint8_t Action;
            int Index; // the shop entry of Buy / Confirm
            bool Enabled;
        };

        static constexpr int MaxButtons = 12;

        void Build();
        void BuildMain();
        void BuildShop();
        void BuildConfirm();
        void Layout();
        void OnButton(int index);

        void AddLine(const std::wstring& text, DWORD color = 0xFFFFFFFF, bool bold = false);
        void AddButton(const std::wstring& text, Command kind, std::uint8_t action = 0, int index = -1, bool enabled = true);

        Page m_page = Page::Main;
        int m_selected = -1;
        std::wstring m_title;
        std::vector<Line> m_lines;
        std::vector<Button> m_buttons;
        std::array<CNewUIMessageBoxButton, MaxButtons> m_buttonControls;
        CNewUIMessageBoxButton m_close;
        int m_middleCount = 1;
    };

    class CIllusionWardenMsgBoxLayout : public TMsgBoxLayout<CIllusionWardenMsgBox>
    {
    public:
        bool SetLayout();
    };
}
