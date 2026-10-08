#include "stdafx.h"
#include "UI/NewUI/Dialogs/IllusionWardenMsgBox.h"

#include <algorithm>

#include "Audio/DSPlaySound.h"
#include "Character/CharacterManager.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Combat/SkillCastTimeOptions.h"
#include "GameLogic/Events/IllusionOfNoria.h"
#include "I18N/All.h"
#include "UI/Legacy/UIControls.h"

namespace
{
    namespace Illusion = GameLogic::Events::IllusionOfNoria;

    constexpr DWORD White = 0xFFFFFFFF;
    constexpr DWORD Gold = 0xFF50C8FF;   // ABGR: 255, 200, 80
    constexpr DWORD Violet = 0xFFFF96C8; // 200, 150, 255
    constexpr DWORD Gray = 0xFFA0A0A0;
    constexpr DWORD Green = 0xFF78FF78;
    constexpr float LineHeight = 14.f;
    constexpr float ButtonStep = 30.f;
    constexpr float TextTop = 34.f;
    constexpr float ButtonWidth = SEASON3B::MSGBOX_BTN_EMPTY_WIDTH + 70.f;

    void SetColor(const DWORD color)
    {
        g_pRenderText->SetTextColor(static_cast<BYTE>(color & 0xFF), static_cast<BYTE>((color >> 8) & 0xFF),
                                    static_cast<BYTE>((color >> 16) & 0xFF), static_cast<BYTE>((color >> 24) & 0xFF));
    }

    std::wstring Format(const wchar_t* format, auto... args)
    {
        wchar_t text[256] = {};
        mu_swprintf_s(text, std::size(text), format, args...);
        return text;
    }

    // the group and number come from the server: only valid indexes are looked up
    const wchar_t* ItemName(const int group, const int number)
    {
        if (group < 0 || group >= MAX_ITEM_TYPE || number < 0 || number >= MAX_ITEM_INDEX)
        {
            return L"?";
        }

        return ItemAttribute[group * MAX_ITEM_INDEX + number].Name;
    }

    const wchar_t* SkillName(const int skill)
    {
        return skill > 0 && skill < MAX_SKILLS ? SkillAttribute[skill].Name : L"-";
    }

    const wchar_t* MonsterName(const int number)
    {
        if (const wchar_t* name = Illusion::MonsterName(number))
        {
            return name;
        }

        return getMonsterName(number);
    }
}

bool SEASON3B::CIllusionWardenMsgBoxLayout::SetLayout()
{
    CIllusionWardenMsgBox* pMsgBox = GetMsgBox();
    return pMsgBox != nullptr && pMsgBox->Create();
}

bool SEASON3B::CIllusionWardenMsgBox::Create(const float fPriority)
{
    AddCallbackFunc(CIllusionWardenMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CIllusionWardenMsgBox::CloseBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    CNewUIMessageBoxBase::Create(static_cast<int>(SCREEN_WIDTH / 2 - MSGBOX_WIDTH / 2), 40, static_cast<int>(MSGBOX_WIDTH),
                                 static_cast<int>(MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT), fPriority);
    m_page = Page::Main;
    Build();
    return true;
}

void SEASON3B::CIllusionWardenMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

void SEASON3B::CIllusionWardenMsgBox::AddLine(const std::wstring& text, const DWORD color, const bool bold)
{
    m_lines.push_back({ text, color, bold });
}

void SEASON3B::CIllusionWardenMsgBox::AddButton(const std::wstring& text, const Command kind, const std::uint8_t action, const int index, const bool enabled)
{
    if (static_cast<int>(m_buttons.size()) < MaxButtons)
    {
        m_buttons.push_back({ text, kind, action, index, enabled });
    }
}

void SEASON3B::CIllusionWardenMsgBox::Build()
{
    m_lines.clear();
    m_buttons.clear();
    m_title = I18N::Game::WardenEldrin;
    switch (m_page)
    {
    case Page::Shop:
        BuildShop();
        break;
    case Page::Confirm:
        BuildConfirm();
        break;
    default:
        BuildMain();
        break;
    }

    Layout();
}

void SEASON3B::CIllusionWardenMsgBox::BuildMain()
{
    const auto& info = Illusion::GetInfo();
    if (!info.InIllusion)
    {
        switch (info.QuestState)
        {
        case 0:
            AddLine(I18N::Game::TheWhistleOfTheVeilWasStolen);
            AddLine(I18N::Game::CondraTheOutcastOfKarutanHasIt);
            AddLine(I18N::Game::BringItBackAndIOpenTheVeilForYou);
            AddLine(Format(I18N::Game::ResetsNeededDYouHaveD, info.RequiredResets, info.Resets), info.Resets >= info.RequiredResets ? Green : Gray);
            AddButton(I18N::Game::AcceptTheQuest, Command::Action, Illusion::AcceptQuest, -1, info.Resets >= info.RequiredResets);
            break;
        case 1:
            AddLine(I18N::Game::HuntCondraInKarutan2ForTheWhistle);
            AddLine(info.HasWhistle ? I18N::Game::YouCarryTheWhistleOfTheVeil : I18N::Game::OnlyTheOneWhoTookTheQuestFindsIt, info.HasWhistle ? Green : Gray);
            AddButton(I18N::Game::GiveTheWhistle, Command::Action, Illusion::TurnIn, -1, info.HasWhistle);
            break;
        default:
            AddLine(I18N::Game::TheVeilIsOpenForYou, Violet);
            AddButton(I18N::Game::EnterTheIllusion, Command::Action, Illusion::Enter, -1, info.Resets >= info.RequiredResets);
            break;
        }
    }
    else
    {
        // the daily quest
        switch (info.DailyState)
        {
        case 0:
            AddLine(Format(I18N::Game::DailyQuestDKillsOfEachOf3Monsters, info.DailyKillsNeeded));
            AddButton(I18N::Game::TakeTheDailyQuest, Command::Action, Illusion::TakeDaily);
            break;
        case 1:
        case 2:
            AddLine(I18N::Game::DailyQuest, Gold, true);
            for (const auto& monster : info.Daily)
            {
                AddLine(Format(I18N::Game::LsDD, MonsterName(monster.Number), std::min(monster.Kills, info.DailyKillsNeeded), info.DailyKillsNeeded),
                        monster.Kills >= info.DailyKillsNeeded ? Green : White);
            }

            if (info.DailyState == 2)
            {
                AddButton(I18N::Game::ClaimTheReward, Command::Action, Illusion::ClaimDaily);
            }

            break;
        default:
            AddLine(Format(I18N::Game::DailyQuestDoneNextInDHDMin, static_cast<int>(info.SecondsUntilNextDay / 3600), static_cast<int>(info.SecondsUntilNextDay % 3600 / 60)), Gray);
            break;
        }
    }

    // the reset of the harmony option of the equipped weapons
    const int family = gCharacterManager.GetBaseClass(CharacterAttribute->Class) * 4;
    for (const auto& weapon : info.Weapons)
    {
        const int skill = GameLogic::Combat::SkillCastTime::OptionSkill(weapon.Group, weapon.Number, weapon.Option, family);
        AddLine(L" ");
        AddLine(Format(I18N::Game::LsLs, ItemName(weapon.Group, weapon.Number), SkillName(skill)), Violet);
        std::wstring price = Format(I18N::Game::ResetDLsDLs, weapon.Jewels, ItemName(14, 42), weapon.LesserStones, ItemName(14, 43));
        if (weapon.GreaterStones > 0)
        {
            price += Format(I18N::Game::DLs, weapon.GreaterStones, ItemName(14, 44));
        }

        AddLine(price, weapon.CanPay ? White : Gray);
        AddButton(Format(I18N::Game::ResetLs, ItemName(weapon.Group, weapon.Number)), Command::Action,
                  static_cast<std::uint8_t>(Illusion::ResetOption + weapon.Slot), -1, weapon.CanPay);
    }

    if (info.QuestState == 2)
    {
        AddButton(Format(I18N::Game::ShopU, info.Shards), Command::Shop);
    }

    if (info.InIllusion)
    {
        AddButton(I18N::Game::ReturnToNoria, Command::Action, Illusion::Return);
    }
}

void SEASON3B::CIllusionWardenMsgBox::BuildShop()
{
    const auto& info = Illusion::GetInfo();
    AddLine(Format(I18N::Game::IllusionShardsU, info.Shards), Violet, true);
    for (int i = 0; i < static_cast<int>(info.Exchange.size()); ++i)
    {
        const auto& entry = info.Exchange[i];
        AddButton(Format(I18N::Game::LsD, ItemName(entry.Group, entry.Number), entry.Price), Command::Buy, 0, i,
                  info.Shards >= static_cast<std::uint32_t>(entry.Price));
    }

    AddButton(I18N::Game::Back, Command::Back);
}

void SEASON3B::CIllusionWardenMsgBox::BuildConfirm()
{
    const auto& info = Illusion::GetInfo();
    if (m_selected < 0 || m_selected >= static_cast<int>(info.Exchange.size()))
    {
        m_page = Page::Shop;
        BuildShop();
        return;
    }

    const auto& entry = info.Exchange[m_selected];
    AddLine(I18N::Game::BuyThisItem, Gold, true);
    AddLine(ItemName(entry.Group, entry.Number), Violet);
    AddLine(Format(I18N::Game::ForDIllusionShards, entry.Price));
    AddButton(I18N::Game::BuyIt, Command::Confirm, 0, m_selected);
    AddButton(I18N::Game::Back, Command::Shop);
}

void SEASON3B::CIllusionWardenMsgBox::Layout()
{
    const float textHeight = TextTop + m_lines.size() * LineHeight + 8.f;
    const float buttonsHeight = m_buttons.size() * ButtonStep;
    const float height = textHeight + buttonsHeight + MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK + 10.f;
    m_middleCount = std::max(1, static_cast<int>(std::ceil((height - MSGBOX_TOP_HEIGHT - MSGBOX_BOTTOM_HEIGHT) / MSGBOX_MIDDLE_HEIGHT)));
    SetSize(static_cast<int>(MSGBOX_WIDTH), static_cast<int>(MSGBOX_TOP_HEIGHT + m_middleCount * MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT));

    const float x = GetPos().x + (GetSize().cx - ButtonWidth) / 2.f;
    float y = GetPos().y + textHeight;
    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
    {
        auto& control = m_buttonControls[i];
        control.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, ButtonWidth, MSGBOX_BTN_EMPTY_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
        control.SetText(m_buttons[i].Text.c_str());
        control.SetEnable(m_buttons[i].Enabled);
        y += ButtonStep;
    }

    const float closeX = GetPos().x + (GetSize().cx - MSGBOX_BTN_EMPTY_SMALL_WIDTH) / 2.f;
    const float closeY = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_close.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, closeX, closeY, MSGBOX_BTN_EMPTY_SMALL_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_close.SetText(I18N::Game::Close388);
}

bool SEASON3B::CIllusionWardenMsgBox::Update()
{
    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
    {
        m_buttonControls[i].Update();
    }

    m_close.Update();
    return true;
}

bool SEASON3B::CIllusionWardenMsgBox::Render()
{
    EnableAlphaTest();
    float x = GetPos().x;
    float y = GetPos().y + 2.f;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, GetSize().cx - MSGBOX_BACK_BLANK_WIDTH, GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT);
    y = GetPos().y;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, MSGBOX_WIDTH, MSGBOX_TOP_HEIGHT);
    y += MSGBOX_TOP_HEIGHT;
    for (int i = 0; i < m_middleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, MSGBOX_WIDTH, MSGBOX_MIDDLE_HEIGHT);
        y += MSGBOX_MIDDLE_HEIGHT;
    }

    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, MSGBOX_WIDTH, MSGBOX_BOTTOM_HEIGHT);

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetFont(g_hFontBold);
    SetColor(Gold);
    g_pRenderText->RenderText(GetPos().x + 10.f, GetPos().y + 10.f, m_title.c_str(), MSGBOX_WIDTH - 20.f, 0, RT3_SORT_CENTER);
    float lineY = GetPos().y + TextTop;
    for (const auto& line : m_lines)
    {
        g_pRenderText->SetFont(line.Bold ? g_hFontBold : g_hFont);
        SetColor(line.Color);
        g_pRenderText->RenderText(GetPos().x + 10.f, lineY, line.Text.c_str(), MSGBOX_WIDTH - 20.f, 0, RT3_SORT_CENTER);
        lineY += LineHeight;
    }

    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
    {
        m_buttonControls[i].Render();
    }

    m_close.Render();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CIllusionWardenMsgBox::OnButton(const int index)
{
    const Button button = m_buttons[index];
    if (!button.Enabled)
    {
        return;
    }

    PlayBuffer(SOUND_CLICK01);
    switch (button.Kind)
    {
    case Command::Action:
        Illusion::SendAction(button.Action);
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_DESTROY);
        return;
    case Command::Shop:
        m_page = Page::Shop;
        break;
    case Command::Back:
        m_page = Page::Main;
        break;
    case Command::Buy:
        m_selected = button.Index;
        m_page = Page::Confirm;
        break;
    case Command::Confirm:
        if (button.Index >= 0 && button.Index < static_cast<int>(Illusion::GetInfo().Exchange.size()))
        {
            const auto& entry = Illusion::GetInfo().Exchange[button.Index];
            Illusion::SendBuy(entry.Group, entry.Number);
        }

        g_MessageBox->SendEvent(this, MSGBOX_EVENT_DESTROY);
        return;
    }

    Build();
}

SEASON3B::CALLBACK_RESULT SEASON3B::CIllusionWardenMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CIllusionWardenMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    for (int i = 0; i < static_cast<int>(pMsgBox->m_buttons.size()); ++i)
    {
        if (pMsgBox->m_buttonControls[i].IsMouseIn())
        {
            pMsgBox->OnButton(i);
            return CALLBACK_BREAK;
        }
    }

    if (pMsgBox->m_close.IsMouseIn())
    {
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
        return CALLBACK_BREAK;
    }

    return CALLBACK_CONTINUE;
}

SEASON3B::CALLBACK_RESULT SEASON3B::CIllusionWardenMsgBox::CloseBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}
