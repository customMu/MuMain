#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"

namespace SEASON3B
{
    // The dialog of Warden Eldrin (Illusion of Noria, GameLogic/Events/IllusionOfNoria.h): the quest of the whistle, the
    // entry, the daily quest, the reset of the harmony option of the equipped weapons and, after the quest, a random
    // stone for Illusion Shards (asks for a confirmation).
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
            Confirm,
        };

        // what a button does
        enum class Command
        {
            Action,  // sends the action to the server and closes
            Back,    // back to the main page
            Buy,     // asks for the confirmation of the random stone
            Confirm, // sends the request of the random stone
        };

        // what a line of the dialog is
        enum class LineKind
        {
            Text,
            Header,   // a section: gold, a thin line under it
            Quote,    // what the warden says: light violet, centred
            Progress, // a bar behind the text (the kills of the daily quest)
        };

        struct Line
        {
            std::wstring Text;
            DWORD Color;
            bool Bold;
            LineKind Kind = LineKind::Text;
            float Progress = 0.f; // 0..1 of a progress line
        };

        struct Button
        {
            std::wstring Text;
            Command Kind;
            std::uint8_t Action;
            int Index; // the entry of the price list (FB 12) of Buy / Confirm
            bool Enabled;
            float X = 0.f; // set by Layout
            float Y = 0.f;
        };

        static constexpr int MaxButtons = 12;

        void Build();
        void BuildMain();
        void BuildConfirm();
        void Layout();
        void OnButton(int index);

        void AddLine(const std::wstring& text, DWORD color = 0xFFFFFFFF, bool bold = false);
        void AddHeader(const std::wstring& text);
        void AddQuote(const std::wstring& text);
        void AddProgress(const std::wstring& text, int value, int maximum);
        void RenderLines();
        void RenderFrame();
        static void RenderButton(const std::wstring& text, float x, float y, float width, bool enabled);
        void AddButton(const std::wstring& text, Command kind, std::uint8_t action = 0, int index = -1, bool enabled = true);

        Page m_page = Page::Main;
        int m_selected = -1;
        std::wstring m_title;
        std::vector<Line> m_lines;
        std::vector<Button> m_buttons;
        float m_closeX = 0.f;
        float m_closeY = 0.f;
    };

    class CIllusionWardenMsgBoxLayout : public TMsgBoxLayout<CIllusionWardenMsgBox>
    {
    public:
        bool SetLayout();
    };
}
