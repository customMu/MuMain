// NewUIHeroPositionInfo.cpp: implementation of the CNewUIHeroPositionInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/HUD/NewUIHeroPositionInfo.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/NewUI/NewUISystem.h"
#include "World/MapInfra/MapManager.h"
#include "MUHelper/MuHelper.h"
#include "Engine/Object/ZzzInventory.h"

using namespace SEASON3B;

CNewUIHeroPositionInfo::CNewUIHeroPositionInfo()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_CurHeroPosition.x = m_CurHeroPosition.y = 0;
}

CNewUIHeroPositionInfo::~CNewUIHeroPositionInfo()
{
    Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIHeroPositionInfo::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_HERO_POSITION_INFO, this);

    WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.2f));
    if (WindowWidth > 800)
    {
        WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.4f));
    }

    SetPos(x, y);
    LoadImages();

    SetButtonInfo(
        &m_BtnConfig,
        IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3,
        x + WidenX + 41,
        y,
        18,
        13,
        1,
        0,
        1,
        1u,
        nullptr,
        &I18N::Game::OfficialMUHelperSetting,
        0);

    SetButtonInfo(
        &m_BtnStart,
        IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4,
        x + WidenX + 59,
        y,
        18,
        13,
        1,
        0,
        1,
        1u,
        nullptr,
        &I18N::Game::StartOfficialMUHelper,
        0);

    SetButtonInfo(
        &m_BtnStop,
        IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5,
        x + WidenX + 59,
        y,
        18,
        13,
        1,
        0,
        1,
        1u,
        nullptr,
        &I18N::Game::StopOfficialMUHelper,
        0);

    MoveTextTipPos(&m_BtnConfig, -20, 9);
    MoveTextTipPos(&m_BtnStart, -20, 9);
    MoveTextTipPos(&m_BtnStop, -20, 9);

    Show(true);

    return true;
}

void CNewUIHeroPositionInfo::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIHeroPositionInfo::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIHeroPositionInfo::BtnProcess()
{
    if (m_BtnConfig.UpdateMouseEvent())
    {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MUHELPER);
        PlayBuffer(SOUND_CLICK01);
        return true;
    }

    if (m_BtnStart.UpdateMouseEvent())
    {
        MUHelper::g_MuHelper.Toggle();

        PlayBuffer(SOUND_CLICK01);
        return true;
    }

    return false;
}

bool CNewUIHeroPositionInfo::UpdateMouseEvent()
{
    if (true == BtnProcess())
    {
        return false;
    }

    int Width = HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX + 73;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, Width, HERO_POSITION_INFO_BASE_WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool CNewUIHeroPositionInfo::UpdateKeyEvent()
{
    return true;
}

bool CNewUIHeroPositionInfo::Update()
{
    if ((IsVisible() == true) && (Hero != NULL))
    {
        m_CurHeroPosition.x = (Hero->PositionX);
        m_CurHeroPosition.y = (Hero->PositionY);
    }

    return true;
}

bool CNewUIHeroPositionInfo::Render()
{
    wchar_t szText[255] = {};

    EnableAlphaTest();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW, m_Pos.x, m_Pos.y, float(HERO_POSITION_INFO_BASEA_WINDOW_WIDTH), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));

    RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1, m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH, m_Pos.y, float(WidenX), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT), 0.1f, 0.f, 22.4f / 32.f, 25.f / 32.f);

    RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX, m_Pos.y, 73.f, 20.f);
    //--
    m_BtnConfig.Render();

    MUHelper::g_MuHelper.IsActive() ? m_BtnStop.Render() : m_BtnStart.Render();
    //--
    mu_swprintf(szText, L"%ls (%d , %d)", gMapManager.GetMapName(gMapManager.WorldActive), m_CurHeroPosition.x, m_CurHeroPosition.y);

    g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 5, szText, WidenX + 20, 13 - 4, RT3_SORT_CENTER);

    RenderHelperStats();

    DisableAlphaBlend();
    return true;
}

namespace
{
    // 1234 -> "1234", 12345 -> "12.3k", 12345678 -> "12.3M"
    void FormatCompact(wchar_t* buffer, size_t size, int64_t value)
    {
        if (value >= 1000000000)
            swprintf(buffer, size, L"%.2fB", value / 1e9);
        else if (value >= 1000000)
            swprintf(buffer, size, L"%.1fM", value / 1e6);
        else if (value >= 10000)
            swprintf(buffer, size, L"%.1fk", value / 1e3);
        else
            swprintf(buffer, size, L"%lld", static_cast<long long>(value));
    }

    int64_t PerHour(int64_t value, uint64_t milliseconds)
    {
        return milliseconds < 10000 ? 0 : static_cast<int64_t>(value * 3600000.0 / milliseconds);
    }
}

// Session statistics of the MU Helper (for balance tests): running time, kills, experience and zen with the rate per hour.
// Shown while the helper runs and after it stopped, until the next start.
void CNewUIHeroPositionInfo::RenderHelperStats()
{
    auto& helper = MUHelper::g_MuHelper;
    s_iHelperStatsBottom = 0;
    if (!helper.HasSessionStats())
    {
        return;
    }

    const uint64_t ms = helper.GetSessionMilliseconds();
    const uint64_t seconds = ms / 1000;
    const int64_t kills = helper.GetSessionKills();
    const int64_t experience = helper.GetSessionExperience();
    const int64_t masterExperience = helper.GetSessionMasterExperience();
    const int64_t zen = helper.GetSessionZen();
    const int hitsDealt = helper.GetSessionHitsDealt();
    const int hitsTaken = helper.GetSessionHitsTaken();

    wchar_t value[32], rate[32], extra[32], line[160];
    const int x = static_cast<int>(m_Pos.x) + 2;
    int y = static_cast<int>(m_Pos.y + HERO_POSITION_INFO_BASE_WINDOW_HEIGHT) + 1;
    constexpr int lineHeight = 12;
    constexpr int boxWidth = 220;
    const auto renderLine = [&]()
    {
        g_pRenderText->RenderText(x, y, line, boxWidth, 0, RT3_SORT_LEFT);
        y += lineHeight;
    };

    g_pRenderText->SetBgColor(0, 0, 0, 160);
    g_pRenderText->SetTextColor(helper.IsActive() ? 120 : 200, 255, helper.IsActive() ? 120 : 200, 255);
    mu_swprintf(extra, L"%llu:%02llu:%02llu", seconds / 3600, seconds / 60 % 60, seconds % 60);
    FormatCompact(value, std::size(value), kills);
    FormatCompact(rate, std::size(rate), PerHour(kills, ms));
    mu_swprintf(line, helper.IsActive() ? I18N::Game::HelperLsKillsLsLsH : I18N::Game::HelperStoppedLsKillsLsLsH, extra, value, rate);
    renderLine();

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    FormatCompact(value, std::size(value), experience);
    FormatCompact(rate, std::size(rate), PerHour(experience, ms));
    mu_swprintf(line, I18N::Game::EXPLsLsH, value, rate);
    renderLine();

    if (masterExperience > 0)
    {
        FormatCompact(value, std::size(value), masterExperience);
        FormatCompact(rate, std::size(rate), PerHour(masterExperience, ms));
        mu_swprintf(line, I18N::Game::MasterEXPLsLsH, value, rate);
        renderLine();
    }

    FormatCompact(value, std::size(value), zen);
    FormatCompact(rate, std::size(rate), PerHour(zen, ms));
    FormatCompact(extra, std::size(extra), helper.GetTotalCost());
    mu_swprintf(line, I18N::Game::ZenLsLsHHelperCostLs, value, rate, extra);
    renderLine();

    // average per second over the session time
    const auto perSecond = [ms](int64_t total) { return ms < 1000 ? int64_t{ 0 } : static_cast<int64_t>(total * 1000.0 / ms); };

    FormatCompact(value, std::size(value), helper.GetSessionDamageDealt());
    FormatCompact(extra, std::size(extra), perSecond(helper.GetSessionDamageDealt()));
    FormatCompact(rate, std::size(rate), PerHour(helper.GetSessionDamageDealt(), ms));
    mu_swprintf(line, I18N::Game::DamageLsLsSLsHHitsD, value, extra, rate,
                hitsDealt > 0 ? (hitsDealt - helper.GetSessionMissesDealt()) * 100 / hitsDealt : 0);
    renderLine();

    FormatCompact(value, std::size(value), helper.GetSessionDamageTaken());
    FormatCompact(extra, std::size(extra), perSecond(helper.GetSessionDamageTaken()));
    mu_swprintf(line, I18N::Game::TakenLsLsSMonstersMissedD, value, extra,
                hitsTaken > 0 ? helper.GetSessionMissesTaken() * 100 / hitsTaken : 0);
    renderLine();

    std::wstring deathTime, killer;
    if (helper.GetLastDeath(deathTime, killer))
    {
        g_pRenderText->SetTextColor(255, 120, 120, 255);
        mu_swprintf(line, I18N::Game::LastDeathLsByLs, deathTime.c_str(), killer.c_str());
        renderLine();
        g_pRenderText->SetTextColor(255, 255, 255, 255);
    }

    mu_swprintf(line, I18N::Game::PotionsHPDDDMPDDD,
                helper.GetSessionPotions(false, 0), helper.GetSessionPotions(false, 1), helper.GetSessionPotions(false, 2),
                helper.GetSessionPotions(true, 0), helper.GetSessionPotions(true, 1), helper.GetSessionPotions(true, 2));
    renderLine();

    // Jewels picked up: only the ones which were picked, e.g. "Bless x3, Soul x1" (wrapped every 3 jewels)
    std::wstring jewels;
    int jewelsInLine = 0;
    const auto& jewelTypes = MUHelper::CMuHelper::GetJewelTypes();
    for (int i = 0; i < MUHelper::CMuHelper::JewelTypeCount; ++i)
    {
        const int count = helper.GetSessionJewels(i);
        if (count <= 0)
        {
            continue;
        }

        wchar_t name[64] = {};
        GetItemName(jewelTypes[i], 0, name);
        std::wstring shortName = name;
        if (shortName.rfind(L"Jewel of ", 0) == 0)
        {
            shortName = shortName.substr(9);
        }

        if (jewelsInLine == 3)
        {
            g_pRenderText->SetTextColor(255, 220, 120, 255);
            mu_swprintf(line, I18N::Game::JewelsLs, jewels.c_str());
            renderLine();
            jewels.clear();
            jewelsInLine = 0;
        }

        wchar_t entry[80];
        mu_swprintf(entry, L"%ls%ls x%d", jewels.empty() ? L"" : L", ", shortName.c_str(), count);
        jewels += entry;
        ++jewelsInLine;
    }

    if (!jewels.empty())
    {
        g_pRenderText->SetTextColor(255, 220, 120, 255);
        mu_swprintf(line, I18N::Game::JewelsLs, jewels.c_str());
        renderLine();
    }

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    s_iHelperStatsBottom = y;
    g_pRenderText->SetBgColor(0, 0, 0, 0);
}

float CNewUIHeroPositionInfo::GetLayerDepth()
{
    return 4.3f;
}

void CNewUIHeroPositionInfo::OpenningProcess()
{
}

void CNewUIHeroPositionInfo::ClosingProcess()
{
}

void CNewUIHeroPositionInfo::SetCurHeroPosition(int x, int y)
{
    m_CurHeroPosition.x = x;
    m_CurHeroPosition.y = y;
}

void CNewUIHeroPositionInfo::LoadImages()
{
    LoadBitmap(L"Interface\\Minimap_positionA.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW, GL_LINEAR);
    LoadBitmap(L"Interface\\Minimap_positionB.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1, GL_LINEAR);
    LoadBitmap(L"Interface\\MacroUI\\Minimap_positionC.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, GL_LINEAR);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_Setup.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_Start.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4, GL_LINEAR);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_Stop.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5, GL_LINEAR);
}

void CNewUIHeroPositionInfo::UnloadImages()
{
    DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW);
    DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1);
    DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2);
}

void CNewUIHeroPositionInfo::SetButtonInfo(CNewUIButton* m_Btn, int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt, const wchar_t* const* btnameSlot, const wchar_t* const* tooltipSlot, bool istoppos)
{
    m_Btn->ChangeButtonImgState(1, imgindex, overflg, isimgwidth, bClickEffect);
    m_Btn->ChangeButtonInfo(x, y, sx, sy);

    if (btnameSlot != nullptr) m_Btn->ChangeText(btnameSlot);
    if (tooltipSlot != nullptr) m_Btn->ChangeToolTipText(tooltipSlot, istoppos);

    if (MoveTxt)
    {
        m_Btn->MoveTextPos(0, -1);
    }
}

void CNewUIHeroPositionInfo::MoveTextTipPos(CNewUIButton* m_Btn, int iX, int iY)
{
    m_Btn->MoveTextTipPos(iX, iY);
}
