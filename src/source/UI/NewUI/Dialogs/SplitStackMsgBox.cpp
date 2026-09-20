#include "stdafx.h"
#include "UI/NewUI/Dialogs/SplitStackMsgBox.h"
#include "Audio/DSPlaySound.h"
#include "GameLogic/Commands/ChatCommandCatalog.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "GameLogic/Items/StackableJewels.h"
#include "I18N/All.h"
#include "UI/NewUI/NewUISystem.h"

using namespace SEASON3B;

namespace
{
// The order of the buttons: first the row of "-1" and "+1", then the row of the fixed amounts.
enum QuickButton
{
    QUICK_MINUS = 0,
    QUICK_PLUS,
    QUICK_TEN,
    QUICK_FIFTY,
    QUICK_HUNDRED,
    QUICK_BUTTON_COUNT,
};

// Amounts which the buttons of the second row set.
constexpr int PRESET_AMOUNTS[] = { 10, 50, 100 };
constexpr int MINIMUM_SPLIT_AMOUNT = 1;

// The label of a button is not a sentence, so it doesn't need to be translated.
const wchar_t* const QUICK_BUTTON_LABELS[QUICK_BUTTON_COUNT] = { L"-1", L"+1", L"10", L"50", L"100" };
constexpr int QUICK_ROW_SIZES[] = { 2, 3 };
constexpr int QUICK_ROW_COUNT = 2;

// The stack keeps at least one piece, so at most (stack size - 1) pieces can be split off.
// Uses the live stack, because the amount could have changed since the popup was opened.
// Returns 0 if the stack isn't there anymore or can't be split.
int GetLiveSplitLimit(int inventorySlot)
{
    const ITEM* pItem = FindInventoryItemBySlot(inventorySlot);
    if (pItem == nullptr || !GameLogic::Items::IsStackableJewel(pItem->Type))
    {
        return 0;
    }

    return std::max<int>(0, static_cast<int>(pItem->Durability) - 1);
}

int AmountAfterQuickButton(int quickIndex, int currentAmount)
{
    switch (quickIndex)
    {
    case QUICK_MINUS:
        return currentAmount - 1;
    case QUICK_PLUS:
        return currentAmount + 1;
    default:
        return PRESET_AMOUNTS[quickIndex - QUICK_TEN];
    }
}

int ReadAmount(CNewUITextInputMsgBox* pMsgBox)
{
    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };
    pMsgBox->GetInputBoxText(strText);
    return _wtoi(strText);
}
} // namespace

int SEASON3B::CSplitStackMsgBoxLayout::s_InventorySlot = -1;

bool SEASON3B::CSplitStackMsgBoxLayout::Open(int inventorySlot)
{
    if (GetLiveSplitLimit(inventorySlot) <= 0)
    {
        return false;
    }

    s_InventorySlot = inventorySlot;
    CreateMessageBox(MSGBOX_LAYOUT_CLASS(CSplitStackMsgBoxLayout));

    // The click which opened the popup must not be handled again, e.g. by the popup itself.
    g_pMyInventory->ResetMouseLButton();
    return true;
}

bool SEASON3B::CSplitStackMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (pMsgBox == nullptr)
        return false;

    const ITEM* pItem = FindInventoryItemBySlot(s_InventorySlot);
    if (pItem == nullptr)
        return false;

    if (!pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, INPUTBOX_WIDTH, INPUTBOX_HEIGHT,
                         INPUTBOX_TEXTLIMIT))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_NUMBERONLY | UIOPTION_PAINTBACK);

    wchar_t strAmount[16];
    mu_swprintf_s(strAmount, L"%d", MINIMUM_SPLIT_AMOUNT);
    pMsgBox->SetInputBoxText(strAmount);

    wchar_t strMsg[256];
    mu_swprintf_s(strMsg, I18N::Game::SplitStackHowManyTotalD, static_cast<int>(pItem->Durability));
    pMsgBox->AddMsg(strMsg);

    // After AddMsg on purpose: the rows take the place of the input field, and AddMsg may have moved that.
    pMsgBox->EnableQuickButtonRows(QUICK_BUTTON_LABELS, QUICK_ROW_SIZES, QUICK_ROW_COUNT);
    pMsgBox->SelectInputBoxText();
    pMsgBox->EnableClearOnFirstClick();
    pMsgBox->AddCallbackFunc(CSplitStackMsgBoxLayout::QuickBtnDown, MSGBOX_EVENT_USER_QUICK);

    pMsgBox->AddCallbackFunc(CSplitStackMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CSplitStackMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CSplitStackMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CSplitStackMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT SEASON3B::CSplitStackMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
        return CALLBACK_CONTINUE;

    const int requested = ReadAmount(pMsgBox);
    if (requested < MINIMUM_SPLIT_AMOUNT)
        return CALLBACK_CONTINUE;

    // Re-check against the live stack, the amount could have changed since the popup was opened.
    const int limit = GetLiveSplitLimit(s_InventorySlot);
    const ITEM* pItem = FindInventoryItemBySlot(s_InventorySlot);
    if (limit <= 0 || pItem == nullptr)
    {
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
        return CALLBACK_BREAK;
    }

    const int amount = std::min(requested, limit);
    const int itemType = pItem->Type;

    // The server checks slot, amount and item type again, so a stale popup can't split the wrong item.
    wchar_t strCommand[64];
    mu_swprintf_s(strCommand, L"/split %d %d %d %d", s_InventorySlot, amount, itemType / MAX_ITEM_INDEX,
                  itemType % MAX_ITEM_INDEX);
    GameLogic::Commands::ChatCommandCatalog::Execute(strCommand);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSplitStackMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner,
                                                              const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner);
}

CALLBACK_RESULT SEASON3B::CSplitStackMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner,
                                                             const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner);
}

CALLBACK_RESULT SEASON3B::CSplitStackMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner,
                                                                 const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSplitStackMsgBoxLayout::QuickBtnDown(class CNewUIMessageBoxBase* pOwner,
                                                                const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
        return CALLBACK_CONTINUE;

    const int limit = GetLiveSplitLimit(s_InventorySlot);
    if (limit <= 0)
        return CALLBACK_CONTINUE;

    const int quickIndex = pMsgBox->GetLastQuickIndex();
    if (quickIndex < 0 || quickIndex >= QUICK_BUTTON_COUNT)
        return CALLBACK_CONTINUE;

    const int current = std::clamp(ReadAmount(pMsgBox), MINIMUM_SPLIT_AMOUNT, limit);
    const int amount = std::clamp(AmountAfterQuickButton(quickIndex, current), MINIMUM_SPLIT_AMOUNT, limit);

    wchar_t strAmount[16];
    mu_swprintf_s(strAmount, L"%d", amount);
    pMsgBox->SetInputBoxText(strAmount);

    PlayBuffer(SOUND_CLICK01);
    return CALLBACK_CONTINUE;
}
