#include "stdafx.h"
#include "UI/NewUI/Dialogs/IllusionWardenMsgBox.h"

#include <algorithm>

#include "Audio/DSPlaySound.h"
#include "Character/CharacterManager.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Combat/SkillCastTimeOptions.h"
#include "GameLogic/Events/IllusionOfNoria.h"
#include "I18N/All.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Core/Utilities/UsefulDef.h"
#include "UI/Legacy/UIControls.h"

namespace
{
    namespace Illusion = GameLogic::Events::IllusionOfNoria;

    constexpr DWORD White = 0xFFFFFFFF;
    constexpr DWORD Gold = 0xFF50C8FF;   // ABGR: 255, 200, 80
    constexpr DWORD Violet = 0xFFFF96C8; // 200, 150, 255
    constexpr DWORD Gray = 0xFFA0A0A0;
    constexpr DWORD Lilac = 0xFFFFC8E6;  // 230, 200, 255: what the warden says
    constexpr DWORD Green = 0xFF78FF78;
    constexpr float LineHeight = 14.f;
    constexpr float TitleHeight = 26.f;
    constexpr float TextTop = 36.f;
    constexpr float ButtonWidth = 150.f;
    constexpr float ButtonHeight = 22.f;
    constexpr float ButtonStep = 27.f;
    constexpr float CloseWidth = 70.f;
    constexpr float CloseHeight = 20.f;

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

    // an entry of the shop: an item, the Veil Ward or the Blessing of the Veil (group 0xFF, the number is the effect)
    const wchar_t* ExchangeName(const Illusion::ExchangeEntry& entry)
    {
        if (entry.Group == Illusion::WardExchangeGroup)
        {
            return entry.Number == Illusion::VeilBlessingEffect ? I18N::Game::BlessingOfTheVeil : I18N::Game::VeilWard;
        }

        return ItemName(entry.Group, entry.Number);
    }

    const wchar_t* ExchangeDescription(const Illusion::ExchangeEntry& entry)
    {
        if (entry.Group == Illusion::WardExchangeGroup)
        {
            return entry.Number == Illusion::VeilBlessingEffect ? I18N::Game::_50DamageToTheMonstersOfTheIllusionOfNoria : I18N::Game::TheGoldenCurseDoesNoDamage;
        }

        return I18N::Game::RaisesTheSkillFixOptionByOneLevel;
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

void SEASON3B::CIllusionWardenMsgBox::AddHeader(const std::wstring& text)
{
    if (!m_lines.empty())
    {
        AddLine(L" ");
    }

    m_lines.push_back({ text, Gold, true, LineKind::Header });
}

void SEASON3B::CIllusionWardenMsgBox::AddQuote(const std::wstring& text)
{
    // the words of the warden are longer than a line: wrapped by the width of the box
    g_pRenderText->SetFont(g_hFont);
    wchar_t rows[4][128] = {};
    const int count = DivideStringByPixel(&rows[0][0], 4, 128, text.c_str(), static_cast<int>(MSGBOX_WIDTH - 40.f), true);
    for (int i = 0; i < count && i < 4; ++i)
    {
        m_lines.push_back({ rows[i], Lilac, false, LineKind::Quote });
    }
}

void SEASON3B::CIllusionWardenMsgBox::AddProgress(const std::wstring& text, const int value, const int maximum)
{
    const float progress = maximum > 0 ? std::clamp(static_cast<float>(value) / static_cast<float>(maximum), 0.f, 1.f) : 0.f;
    m_lines.push_back({ text, progress >= 1.f ? Green : White, false, LineKind::Progress, progress });
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
            AddQuote(I18N::Game::WardenQuoteStolen);
            AddHeader(I18N::Game::TheStolenWhistle);
            AddLine(I18N::Game::TheWhistleOfTheVeilWasStolen);
            AddLine(I18N::Game::CondraTheOutcastOfKarutanHasIt);
            AddLine(I18N::Game::BringItBackAndIOpenTheVeilForYou);
            AddLine(Format(I18N::Game::ResetsNeededDYouHaveD, info.RequiredResets, info.Resets), info.Resets >= info.RequiredResets ? Green : Gray);
            AddButton(I18N::Game::AcceptTheQuest, Command::Action, Illusion::AcceptQuest, -1, info.Resets >= info.RequiredResets);
            break;
        case 1:
            AddQuote(info.HasWhistle ? I18N::Game::WardenQuoteWhistleSings : I18N::Game::WardenQuoteHunt);
            AddHeader(I18N::Game::TheStolenWhistle);
            AddLine(I18N::Game::HuntCondraInKarutan2ForTheWhistle);
            AddLine(info.HasWhistle ? I18N::Game::YouCarryTheWhistleOfTheVeil : I18N::Game::OnlyTheOneWhoTookTheQuestFindsIt, info.HasWhistle ? Green : Gray);
            AddButton(I18N::Game::GiveTheWhistle, Command::Action, Illusion::TurnIn, -1, info.HasWhistle);
            break;
        default:
            AddQuote(I18N::Game::WardenQuoteVeilOpen);
            AddLine(I18N::Game::TheVeilIsOpenForYou, Violet);
            AddButton(I18N::Game::EnterTheIllusion, Command::Action, Illusion::Enter, -1, info.Resets >= info.RequiredResets);
            break;
        }
    }
    else
    {
        AddQuote(I18N::Game::WardenQuoteIllusion);
        AddHeader(I18N::Game::DailyQuest);
        switch (info.DailyState)
        {
        case 0:
            AddLine(Format(I18N::Game::DailyQuestDKillsOfEachOf3Monsters, info.DailyKillsNeeded));
            AddButton(I18N::Game::TakeTheDailyQuest, Command::Action, Illusion::TakeDaily);
            break;
        case 1:
        case 2:
            for (const auto& monster : info.Daily)
            {
                AddProgress(Format(I18N::Game::LsDD, MonsterName(monster.Number), std::min(monster.Kills, info.DailyKillsNeeded), info.DailyKillsNeeded),
                            monster.Kills, info.DailyKillsNeeded);
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

    // the reset of the skill fix option of the equipped weapons
    if (!info.Weapons.empty())
    {
        AddHeader(I18N::Game::SkillFixOption);
    }

    const int family = gCharacterManager.GetBaseClass(CharacterAttribute->Class) * 4;
    for (const auto& weapon : info.Weapons)
    {
        const int skill = GameLogic::Combat::SkillCastTime::OptionSkill(weapon.Group, weapon.Number, weapon.Option, family);
        AddLine(Format(I18N::Game::LsLs, ItemName(weapon.Group, weapon.Number), SkillName(skill)), Violet, true);
        std::wstring price = Format(I18N::Game::ResetDLsDLs, weapon.Jewels, ItemName(14, 195), weapon.LesserStones, ItemName(14, 196));
        if (weapon.GreaterStones > 0)
        {
            price += Format(I18N::Game::DLs, weapon.GreaterStones, ItemName(14, 197));
        }

        AddLine(price, weapon.CanPay ? White : Gray);
        AddButton(Format(I18N::Game::ResetLs, ItemName(weapon.Group, weapon.Number)), Command::Action,
                  static_cast<std::uint8_t>(Illusion::ResetOption + weapon.Slot), -1, weapon.CanPay);
    }

    // the shop: a Lesser Mirage Stone and the Veil Ward for shards; the prices come from the server
    if (info.QuestState == 2 && !info.Exchange.empty())
    {
        AddHeader(I18N::Game::Shop);
        AddProgress(Format(I18N::Game::IllusionShardsUD, info.Shards, info.Exchange.front().Price), static_cast<int>(info.Shards), info.Exchange.front().Price);
        for (int i = 0; i < static_cast<int>(info.Exchange.size()); ++i)
        {
            const auto& entry = info.Exchange[i];
            AddButton(Format(I18N::Game::LsDShards, ExchangeName(entry), entry.Price), Command::Buy, 0, i, info.Shards >= static_cast<std::uint32_t>(entry.Price));
        }
    }

    if (info.InIllusion)
    {
        AddButton(I18N::Game::ReturnToNoria, Command::Action, Illusion::Return);
    }
}

void SEASON3B::CIllusionWardenMsgBox::BuildConfirm()
{
    const auto& info = Illusion::GetInfo();
    if (m_selected < 0 || m_selected >= static_cast<int>(info.Exchange.size()))
    {
        m_page = Page::Main;
        BuildMain();
        return;
    }

    const auto& entry = info.Exchange[m_selected];
    AddLine(ExchangeName(entry), Gold, true);
    AddLine(ExchangeDescription(entry), Violet);
    if (entry.Group == Illusion::WardExchangeGroup)
    {
        AddLine(entry.Number == Illusion::VeilBlessingEffect ? I18N::Game::_1HourWorksTogetherWithTheVeilWard : I18N::Game::_30MinutesWorksTogetherWithTheBlessing, Gray);
    }
    AddLine(Format(I18N::Game::ForDIllusionShards, entry.Price));
    AddButton(I18N::Game::BuyIt, Command::Confirm, 0, m_selected);
    AddButton(I18N::Game::Back, Command::Back);
}

void SEASON3B::CIllusionWardenMsgBox::Layout()
{
    const float textHeight = TextTop + m_lines.size() * LineHeight + 10.f;
    const float height = textHeight + m_buttons.size() * ButtonStep + 14.f + CloseHeight + 14.f;
    SetSize(static_cast<int>(MSGBOX_WIDTH), static_cast<int>(height));

    const float x = GetPos().x + (MSGBOX_WIDTH - ButtonWidth) / 2.f;
    float y = GetPos().y + textHeight;
    for (auto& button : m_buttons)
    {
        button.X = x;
        button.Y = y;
        y += ButtonStep;
    }

    m_closeX = GetPos().x + (MSGBOX_WIDTH - CloseWidth) / 2.f;
    m_closeY = GetPos().y + height - CloseHeight - 12.f;
}

bool SEASON3B::CIllusionWardenMsgBox::Update()
{
    return true;
}

void SEASON3B::CIllusionWardenMsgBox::RenderFrame()
{
    const float x = GetPos().x;
    const float y = GetPos().y;
    const float w = static_cast<float>(GetSize().cx);
    const float h = static_cast<float>(GetSize().cy);

    // a dark violet panel, the band of the title a bit lighter
    RenderColorQuadARGB(x, y, w, h, 0xF0120A1Au);
    RenderColorQuadARGB(x, y, w, TitleHeight, 0xF02A163Cu);
    RenderColorQuadARGB(x + 4.f, y + TitleHeight, w - 8.f, 18.f, 0x40281440u);

    // a gold outer line, a violet inner line and gold corners
    constexpr unsigned int GoldLine = 0xFFB08A3Cu;
    constexpr unsigned int VioletLine = 0x907850B0u;
    RenderColorLineARGB(x, y, x + w, y, 1.f, GoldLine);
    RenderColorLineARGB(x, y + h, x + w, y + h, 1.f, GoldLine);
    RenderColorLineARGB(x, y, x, y + h, 1.f, GoldLine);
    RenderColorLineARGB(x + w, y, x + w, y + h, 1.f, GoldLine);
    RenderColorLineARGB(x + 3.f, y + 3.f, x + w - 3.f, y + 3.f, 1.f, VioletLine);
    RenderColorLineARGB(x + 3.f, y + h - 3.f, x + w - 3.f, y + h - 3.f, 1.f, VioletLine);
    RenderColorLineARGB(x + 3.f, y + 3.f, x + 3.f, y + h - 3.f, 1.f, VioletLine);
    RenderColorLineARGB(x + w - 3.f, y + 3.f, x + w - 3.f, y + h - 3.f, 1.f, VioletLine);
    RenderColorLineARGB(x + 12.f, y + TitleHeight, x + w - 12.f, y + TitleHeight, 1.f, GoldLine);
    for (const auto& [cx, cy] : { std::pair{ x, y }, std::pair{ x + w - 6.f, y }, std::pair{ x, y + h - 6.f }, std::pair{ x + w - 6.f, y + h - 6.f } })
    {
        RenderColorQuadARGB(cx, cy, 6.f, 6.f, 0xFFD8B060u);
    }
}

void SEASON3B::CIllusionWardenMsgBox::RenderButton(const std::wstring& text, const float x, const float y, const float width, const bool enabled)
{
    const bool hover = enabled && SEASON3B::CheckMouseIn(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(ButtonHeight));
    RenderColorQuadARGB(x, y, width, ButtonHeight, !enabled ? 0xD0201C24u : hover ? 0xF0503078u : 0xE0302044u);
    RenderColorQuadARGB(x + 1.f, y + 1.f, width - 2.f, ButtonHeight / 2.f - 1.f, hover ? 0x30FFFFFFu : 0x14FFFFFFu); // a gloss on the upper half
    const unsigned int border = !enabled ? 0xFF4A4450u : hover ? 0xFFFFD27Au : 0xFF8C6E3Au;
    RenderColorLineARGB(x, y, x + width, y, 1.f, border);
    RenderColorLineARGB(x, y + ButtonHeight, x + width, y + ButtonHeight, 1.f, border);
    RenderColorLineARGB(x, y, x, y + ButtonHeight, 1.f, border);
    RenderColorLineARGB(x + width, y, x + width, y + ButtonHeight, 1.f, border);

    EnableAlphaTest();
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetFont(hover ? g_hFontBold : g_hFont);
    SetColor(!enabled ? Gray : hover ? 0xFF80E6FFu : White);
    g_pRenderText->RenderText(x, y + (ButtonHeight - 12.f) / 2.f, text.c_str(), width, 0, RT3_SORT_CENTER);
}

bool SEASON3B::CIllusionWardenMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();

    // the name of the warden glows between gold and the violet of the Veil
    const float pulse = sinf(static_cast<float>(WorldTime) * 0.0025f) * 0.5f + 0.5f;
    const auto mix = [pulse](int a, int b) { return static_cast<BYTE>(a + (b - a) * pulse); };
    EnableAlphaTest();
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(mix(255, 210), mix(200, 160), mix(80, 255), 255);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + (TitleHeight - 12.f) / 2.f, m_title.c_str(), MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
    RenderLines();

    for (const auto& button : m_buttons)
    {
        RenderButton(button.Text, button.X, button.Y, ButtonWidth, button.Enabled);
    }

    RenderButton(I18N::Game::Close388, m_closeX, m_closeY, CloseWidth, true);
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CIllusionWardenMsgBox::RenderLines()
{
    const float left = GetPos().x + 18.f;
    const float width = MSGBOX_WIDTH - 36.f;
    float lineY = GetPos().y + TextTop;
    for (const auto& line : m_lines)
    {
        if (line.Kind == LineKind::Progress)
        {
            // a dark track and a violet fill (green when done) behind the text
            RenderColorQuadARGB(left, lineY + 1.f, width, LineHeight - 2.f, 0xA0201030u);
            RenderColorQuadARGB(left, lineY + 1.f, width * line.Progress, LineHeight - 2.f, line.Progress >= 1.f ? 0xC0308040u : 0xC07040B0u);
        }
        else if (line.Kind == LineKind::Header)
        {
            RenderColorLineARGB(left, lineY + LineHeight - 1.f, left + width, lineY + LineHeight - 1.f, 1.f, 0xB0C89640u);
        }

        EnableAlphaTest();
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetFont(line.Bold ? g_hFontBold : g_hFont);
        SetColor(line.Color);
        g_pRenderText->RenderText(GetPos().x + 10.f, lineY, line.Text.c_str(), MSGBOX_WIDTH - 20.f, 0, RT3_SORT_CENTER);
        lineY += LineHeight;
    }
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
    case Command::Back:
        m_page = Page::Main;
        break;
    case Command::Buy:
        m_selected = button.Index;
        m_page = Page::Confirm;
        break;
    case Command::Confirm:
        if (m_selected >= 0 && m_selected < static_cast<int>(Illusion::GetInfo().Exchange.size()))
        {
            const auto& entry = Illusion::GetInfo().Exchange[m_selected];
            Illusion::SendAction(entry.Group != Illusion::WardExchangeGroup ? Illusion::BuyLesserStone
                                 : entry.Number == Illusion::VeilBlessingEffect ? Illusion::BuyBlessing : Illusion::BuyWard);
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
        const auto& button = pMsgBox->m_buttons[i];
        if (CheckMouseIn(static_cast<int>(button.X), static_cast<int>(button.Y), static_cast<int>(ButtonWidth), static_cast<int>(ButtonHeight)))
        {
            // the click belongs to the dialog: the inventory under it picks items on the release too
            g_pNewKeyInput->SetKeyState(VK_LBUTTON, SEASON3B::CNewKeyInput::KEY_NONE);
            pMsgBox->OnButton(i);
            return CALLBACK_BREAK;
        }
    }

    if (CheckMouseIn(static_cast<int>(pMsgBox->m_closeX), static_cast<int>(pMsgBox->m_closeY), static_cast<int>(CloseWidth), static_cast<int>(CloseHeight)))
    {
        g_pNewKeyInput->SetKeyState(VK_LBUTTON, SEASON3B::CNewKeyInput::KEY_NONE);
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
