#pragma once

#include "UI/NewUI/Dialogs/NewUICustomMessageBox.h"

namespace SEASON3B
{
    // Popup which asks how many pieces should be split off a stack of jewels into a new stack.
    // It is opened with Shift + click on the stack. The amount can be typed or chosen with the buttons
    // "-", "10", "50", "100" and "+". On OK, the server command "/split" does the work.
    class CSplitStackMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        // Opens the popup for the stack in the given inventory slot.
        // Returns false (and opens nothing) if the slot doesn't hold a stack which can be split.
        static bool Open(int inventorySlot);

        bool SetLayout();

        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        // Changes the amount in the input field. Does NOT confirm anything, the player still presses OK.
        static CALLBACK_RESULT QuickBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner);

        // The layout is default-constructed by the MSGBOX_LAYOUT_CLASS macro and has no other way
        // to learn which stack was clicked, so Open() stores it here.
        static int s_InventorySlot;
    };
}
