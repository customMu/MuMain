// NewUIMyQuestInfoWindow.cpp: implementation of the CNewUIMyQuestInfoWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameLogic/Quests/KillQuest.h"
#include "GameLogic/Events/IllusionOfNoria.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Renderer/MuRenderer.h"

namespace
{
    // The button "take the reward" of the kill quests, in the tab "Quest" (window coordinates); it is placed below the
    // lines of the reward, so its y is set when the window is rendered.
    constexpr int KillQuestButtonX = 50;
    constexpr int KillQuestButtonWidth = 90;
    constexpr int KillQuestButtonHeight = 18;
    int s_killQuestButtonY = 200;
}
#include "UI/NewUI/Quests/NewUIMyQuestInfoWindow.h"
#include "I18N/All.h"

#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Quests/ClassChangeQuests.h"
#include "Character/CharacterManager.h"
#include "Engine/Object/ZzzInventory.h"

namespace
{
    void RenderClassChangeGoal(const POINT& pos);
}
#include "GameLogic/Quests/QuestMng.h"
#include "Audio/DSPlaySound.h"
#include "UI/NewUI/NewUISystem.h"

using namespace SEASON3B;

extern int g_iNumLineMessageBoxCustom;
extern int g_iNumAnswer;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];

SEASON3B::CNewUIMyQuestInfoWindow::CNewUIMyQuestInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUIMyQuestInfoWindow::~CNewUIMyQuestInfoWindow()
{
    Release();
}

bool SEASON3B::CNewUIMyQuestInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MYQUEST, this);

    SetPos(x, y);
    LoadImages();
    SetButtonInfo();
    m_eTabBtnIndex = TAB_QUEST;
    Show(false);

    return true;
}

void SEASON3B::CNewUIMyQuestInfoWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIMyQuestInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_btnQuestOpen.ChangeButtonInfo(m_Pos.x + 50, m_Pos.y + 392, 36, 29);
    m_btnQuestGiveUp.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 392, 36, 29);

    m_CurQuestListBox.SetPosition(m_Pos.x + 9, m_Pos.y + 160);
    m_QuestContentsListBox.SetPosition(m_Pos.x + 9, m_Pos.y + 390);
}

bool SEASON3B::CNewUIMyQuestInfoWindow::UpdateMouseEvent()
{
    if (m_eTabBtnIndex == TAB_QUEST)
    {
        m_CurQuestListBox.DoAction();
        m_QuestContentsListBox.DoAction();
    }

    if (BtnProcess() == true)
    {
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, MYQUESTINFO_WINDOW_WIDTH, MYQUESTINFO_WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame). Hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, SEASON3B::INTERFACE_MYQUEST))
        return true;

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
        return true;
    }

    TAB_BUTTON_INDEX eTabBtnIndex = UpdateTabBtn();
    if (eTabBtnIndex == TAB_QUEST)
    {
        if (0 == m_CurQuestListBox.GetLineNum())
            SetMessage(2825);
        return true;
    }

    if (eTabBtnIndex == TAB_JOB_CHANGE)
    {
        /*		BYTE byState = g_csQuest.getCurrQuestState();
                if (byState == QUEST_NONE || byState == QUEST_NO || byState == QUEST_ERROR)
                    SetMessage(930);
                else if(byState == QUEST_ING )
                    SetMessage(931);
                else if(byState == QUEST_END )
                    SetMessage(932);
                */
        return true;
    }

    if (eTabBtnIndex == TAB_DAILY)
    {
        GameLogic::Events::IllusionOfNoria::SendAction(GameLogic::Events::IllusionOfNoria::State); // a fresh state of the daily quest
        return true;
    }

    if (m_eTabBtnIndex == TAB_QUEST && GameLogic::Quests::KillQuest::IsRewardWaiting()
        && SEASON3B::IsRelease(VK_LBUTTON)
        && SEASON3B::CheckMouseIn(m_Pos.x + KillQuestButtonX, m_Pos.y + s_killQuestButtonY, KillQuestButtonWidth, KillQuestButtonHeight))
    {
        ::PlayBuffer(SOUND_CLICK01);
        GameLogic::Quests::KillQuest::RequestReward();
        return true;
    }

    if (m_eTabBtnIndex == TAB_QUEST)
    {
        if (m_btnQuestOpen.UpdateMouseEvent())
        {
            ::PlayBuffer(SOUND_CLICK01);
            g_pQuestProgressByEtc->SetContents(GetSelQuestIndex());
            g_pNewUISystem->Show(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
            return true;
        }

        if (m_btnQuestGiveUp.UpdateMouseEvent())
        {
            ::PlayBuffer(SOUND_CLICK01);
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CQuestGiveUpMsgBoxLayout));
            return true;
        }
    }
    return false;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTabBtn();
    RenderSubjectTexts();
    m_BtnExit.Render();

    if (m_eTabBtnIndex == TAB_QUEST)
    {
        RenderQuestInfo();
    }
    else if (m_eTabBtnIndex == TAB_JOB_CHANGE)
    {
        RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y + 182, 188.f, 21.f);
        RenderJobChangeContents();
        RenderJobChangeState();
        RenderClassChangeGoal(m_Pos);
    }
    else if (m_eTabBtnIndex == TAB_DAILY)
    {
        RenderDailyInfo();
    }

    DisableAlphaBlend();

    return true;
}

float SEASON3B::CNewUIMyQuestInfoWindow::GetLayerDepth()
{
    return 3.3f;
}

void SEASON3B::CNewUIMyQuestInfoWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_MYQUEST_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_MYQUEST_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_MYQUEST_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_MYQUEST_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_MYQUEST_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_MYQUEST_BTN_EXIT, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_MYQUEST_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_Bt_open.tga", IMAGE_MYQUEST_BTN_OPEN, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_Bt_cast.tga", IMAGE_MYQUEST_BTN_GIVE_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab01.tga", IMAGE_MYQUEST_TAB_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab02.tga", IMAGE_MYQUEST_TAB_SMALL, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab03.tga", IMAGE_MYQUEST_TAB_BIG, GL_LINEAR);
}

void SEASON3B::CNewUIMyQuestInfoWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_MYQUEST_TAB_BIG);
    DeleteBitmap(IMAGE_MYQUEST_TAB_SMALL);
    DeleteBitmap(IMAGE_MYQUEST_TAB_BACK);
    DeleteBitmap(IMAGE_MYQUEST_BTN_GIVE_UP);
    DeleteBitmap(IMAGE_MYQUEST_BTN_OPEN);
    DeleteBitmap(IMAGE_MYQUEST_LINE);
    DeleteBitmap(IMAGE_MYQUEST_BTN_EXIT);
    DeleteBitmap(IMAGE_MYQUEST_BOTTOM);
    DeleteBitmap(IMAGE_MYQUEST_RIGHT);
    DeleteBitmap(IMAGE_MYQUEST_LEFT);
    DeleteBitmap(IMAGE_MYQUEST_TOP);
    DeleteBitmap(IMAGE_MYQUEST_BACK);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderFrame()
{
    RenderImage(IMAGE_MYQUEST_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_MYQUEST_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_MYQUEST_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MYQUEST_RIGHT, m_Pos.x + 190 - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MYQUEST_BOTTOM, m_Pos.x, m_Pos.y + 429 - 45, 190.f, 45.f);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderSubjectTexts()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, L"Quest", 190, 0, RT3_SORT_CENTER);
}

namespace
{
    // The kill quests of the server: the current quest, its progress, the reward, the reward which waits.
    void RenderKillQuest(const POINT& pos)
    {
        using namespace GameLogic::Quests::KillQuest;
        const State& state = GetState();
        // a reward line is up to 191 characters (192 bytes in FB 0D) plus "Reward: "
        wchar_t text[256];
        const int x = static_cast<int>(pos.x) + 23;
        int y = static_cast<int>(pos.y) + 62;
        g_pRenderText->SetBgColor(0);

        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetTextColor(255, 210, 90, 255);
        mu_swprintf(text, I18N::Game::KillQuestsDD, (std::min)(state.number, state.count), state.count);
        g_pRenderText->RenderText(x, y, text, 144, 0, RT3_SORT_CENTER);
        y += 18;

        g_pRenderText->SetFont(g_hFont);
        if (IsAllDone())
        {
            g_pRenderText->SetTextColor(120, 230, 120, 255);
            g_pRenderText->RenderText(x, y, I18N::Game::AllKillQuestsAreCompleted, 144, 0, RT3_SORT_CENTER);
            y += 16;
        }
        else
        {
            g_pRenderText->SetTextColor(230, 230, 230, 255);
            mu_swprintf(text, I18N::Game::KillLs, state.monster.c_str());
            g_pRenderText->RenderText(x, y, text, 144, 0, RT3_SORT_CENTER);
            y += 16;

            // progress bar
            const float share = state.killsNeeded > 0 ? (std::min)(1.f, static_cast<float>(state.kills) / state.killsNeeded) : 0.f;
            RenderColorQuadARGB(static_cast<float>(x), static_cast<float>(y), 144.f, 10.f, 0xFF202020u);
            RenderColorQuadARGB(static_cast<float>(x), static_cast<float>(y), 144.f * share, 10.f, state.rewardWaiting ? 0xFF40C040u : 0xFFD0A030u);
            mu::GetRenderer().SetTexture2D(true);
            mu_swprintf(text, L"%d / %d", state.kills, state.killsNeeded);
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->RenderText(x, y, text, 144, 10, RT3_SORT_CENTER);
            y += 16;
        }

        if (state.questPoints > 0)
        {
            g_pRenderText->SetTextColor(181, 181, 181, 255);
            mu_swprintf(text, I18N::Game::StatPointsFromQuestsD, state.questPoints);
            g_pRenderText->RenderText(x, y, text, 144, 0, RT3_SORT_CENTER);
        }

        if (IsAllDone())
        {
            return;
        }

        // the reward below the line of the window: the stat points, then one item per line (the server separates them by '\n')
        y = static_cast<int>(pos.y) + 186;
        g_pRenderText->SetTextColor(200, 200, 255, 255);
        bool first = true;
        if (state.rewardPoints > 0)
        {
            mu_swprintf(text, I18N::Game::RewardDStatPoints, state.rewardPoints);
            g_pRenderText->RenderText(x, y, text, 144, 0, RT3_SORT_CENTER);
            y += 13;
            first = false;
        }

        std::size_t begin = 0;
        while (begin < state.reward.size())
        {
            auto lineEnd = state.reward.find(L'\n', begin);
            if (lineEnd == std::wstring::npos)
            {
                lineEnd = state.reward.size();
            }

            const std::wstring line = state.reward.substr(begin, lineEnd - begin);
            begin = lineEnd + 1;
            if (line.empty())
            {
                continue;
            }

            if (first)
            {
                mu_swprintf(text, I18N::Game::RewardLs, line.c_str());
                first = false;
            }
            else
            {
                mu_swprintf(text, L"%ls", line.c_str());
            }

            g_pRenderText->RenderText(x, y, text, 144, 0, RT3_SORT_CENTER);
            y += 13;
        }

        if (state.rewardWaiting)
        {
            y += 4;
            g_pRenderText->SetTextColor(255, 90, 80, 255);
            g_pRenderText->RenderText(x, y, I18N::Game::TheRewardWaitsFreeSpaceInTheInventory, 144, 0, RT3_SORT_CENTER);
            s_killQuestButtonY = y + 15 - static_cast<int>(pos.y);
            const int bx = static_cast<int>(pos.x) + KillQuestButtonX;
            const int by = static_cast<int>(pos.y) + s_killQuestButtonY;
            const bool hover = SEASON3B::CheckMouseIn(bx, by, KillQuestButtonWidth, KillQuestButtonHeight);
            RenderColorQuadARGB(static_cast<float>(bx), static_cast<float>(by), static_cast<float>(KillQuestButtonWidth), static_cast<float>(KillQuestButtonHeight), hover ? 0xFF8A6A20u : 0xFF5A4410u);
            mu::GetRenderer().SetTexture2D(true);
            g_pRenderText->SetFont(g_hFontBold);
            g_pRenderText->SetTextColor(255, 230, 150, 255);
            g_pRenderText->RenderText(bx, by + 3, I18N::Game::TakeReward, KillQuestButtonWidth, 0, RT3_SORT_CENTER);
            g_pRenderText->SetFont(g_hFont);
        }
    }
}

namespace
{
    // The class change quest the hero works on: the first one which isn't finished, in the order of the class
    // (the same as CSQuest::setQuestLists); -1 when all are done.
    int GetCurrentClassChangeQuest()
    {
        const auto baseClass = gCharacterManager.GetBaseClass(Hero->Class);
        const bool onlyThirdClass = baseClass == CLASS_DARK || baseClass == CLASS_DARK_LORD || baseClass == CLASS_RAGEFIGHTER;
        for (int quest = onlyThirdClass ? QUEST_3RD_CHANGE_UP_1 : QUEST_CHANGE_UP_1; quest < QUEST_LIST_END; ++quest)
        {
            if (quest == QUEST_COMBO && baseClass != CLASS_KNIGHT)
            {
                continue;
            }

            if (g_csQuest.getQuestState2(quest) != QUEST_END)
            {
                return quest;
            }
        }

        return -1;
    }

    // What the class change quest needs and where it is: items (count in the inventory, monsters, map, chance)
    // or kills, the requirements and the NPC.
    void RenderClassChangeGoal(const POINT& pos)
    {
        using namespace GameLogic::Quests::ClassChange;
        const int quest = GetCurrentClassChangeQuest();
        const QuestInfo* info = quest >= 0 ? FindQuest(quest) : nullptr;
        if (info == nullptr || CharacterAttribute == nullptr)
        {
            return;
        }

        const auto baseClass = gCharacterManager.GetBaseClass(Hero->Class);
        const bool active = g_csQuest.getQuestState2(quest) == QUEST_ING;
        const int x = static_cast<int>(pos.x) + 21;
        const int width = 148;
        int y = static_cast<int>(pos.y) + 200;
        wchar_t text[160];
        g_pRenderText->SetBgColor(0);

        for (const auto& goal : ItemGoals)
        {
            if (goal.Quest != quest || (goal.BaseClass != AnyClass && goal.BaseClass != baseClass))
            {
                continue;
            }

            const int missing = g_csQuest.FindQuestItemsInInven(goal.ItemType, 1, goal.ItemLevel);
            wchar_t name[64] = {};
            GetItemName(goal.ItemType, goal.ItemLevel, name);
            g_pRenderText->SetFont(g_hFontBold);
            if (missing == 0)
            {
                g_pRenderText->SetTextColor(120, 230, 120, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(255, 210, 90, 255);
            }

            g_pRenderText->RenderText(x, y, name, width, 0, RT3_SORT_CENTER);
            y += 13;

            g_pRenderText->SetFont(g_hFont);
            g_pRenderText->SetTextColor(230, 230, 230, 255);
            if (goal.MonsterLevel > 0)
            {
                mu_swprintf(text, I18N::Game::InTheInventoryDD, 1 - missing, 1);
                g_pRenderText->RenderText(x, y, text, width, 0, RT3_SORT_CENTER);
                y += 13;
                mu_swprintf(text, I18N::Game::MonstersLevelD, goal.MonsterLevel);
                g_pRenderText->RenderText(x, y, text, width, 0, RT3_SORT_CENTER);
                y += 13;
                g_pRenderText->RenderText(x, y, goal.Place, width, 0, RT3_SORT_CENTER);
                y += 13;
                g_pRenderText->SetTextColor(200, 200, 255, 255);
                mu_swprintf(text, I18N::Game::ChanceLsPerKill, goal.Chance);
                g_pRenderText->RenderText(x, y, text, width, 0, RT3_SORT_CENTER);
                y += 13;
                g_pRenderText->SetTextColor(181, 181, 181, 255);
                g_pRenderText->RenderText(x, y, I18N::Game::DropsOnlyWhileTheQuestIsActive, width, 0, RT3_SORT_CENTER);
                y += 13;
            }
            else
            {
                mu_swprintf(text, I18N::Game::BossLs, goal.Place);
                g_pRenderText->RenderText(x, y, text, width, 0, RT3_SORT_CENTER);
                y += 13;
            }
        }

        for (const auto& goal : KillGoals)
        {
            if (goal.Quest != quest)
            {
                continue;
            }

            const int kills = active ? g_csQuest.GetKillMobCount(goal.MonsterNumber) : -1;
            g_pRenderText->SetFont(g_hFont);
            if (kills >= goal.Count)
            {
                g_pRenderText->SetTextColor(120, 230, 120, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(255, 210, 90, 255);
            }

            if (kills >= 0)
            {
                mu_swprintf(text, I18N::Game::KillLsDD, goal.Monster, (std::min)(kills, goal.Count), goal.Count);
            }
            else
            {
                mu_swprintf(text, I18N::Game::KillLsD, goal.Monster, goal.Count);
            }

            g_pRenderText->RenderText(x, y, text, width, 0, RT3_SORT_CENTER);
            y += 13;
        }

        for (const auto& goal : KillGoals)
        {
            if (goal.Quest == quest)
            {
                g_pRenderText->SetTextColor(230, 230, 230, 255);
                g_pRenderText->RenderText(x, y, goal.Place, width, 0, RT3_SORT_CENTER);
                break;
            }
        }

        // the NPC and, before the quest is taken, the requirements
        y = static_cast<int>(pos.y) + 326;
        g_pRenderText->SetFont(g_hFont);
        if (!active)
        {
            const bool levelOk = CharacterAttribute->Level >= info->MinimumLevel;
            const bool resetsOk = static_cast<int>(CharacterAttribute->Resets) >= info->MinimumResets;
            if (levelOk && resetsOk)
            {
                g_pRenderText->SetTextColor(120, 230, 120, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(255, 90, 80, 255);
            }

            if (info->MinimumResets > 1)
            {
                mu_swprintf(text, I18N::Game::LevelDAndDResetsNeeded, info->MinimumLevel, info->MinimumResets);
            }
            else if (info->MinimumResets == 1)
            {
                mu_swprintf(text, I18N::Game::LevelDAnd1ResetNeeded, info->MinimumLevel);
            }
            else
            {
                mu_swprintf(text, I18N::Game::LevelDNeeded, info->MinimumLevel);
            }

            g_pRenderText->RenderText(x, y, text, width, 0, RT3_SORT_CENTER);
            y += 13;
        }

        g_pRenderText->SetTextColor(36, 242, 252, 255);
        mu_swprintf(text, active ? I18N::Game::ReturnToLs : I18N::Game::TalkToLs, info->Npc);
        g_pRenderText->RenderText(x, y, text, width, 0, RT3_SORT_CENTER);
        y += 13;
        g_pRenderText->SetTextColor(181, 181, 181, 255);
        g_pRenderText->RenderText(x, y, info->NpcPlace, width, 0, RT3_SORT_CENTER);
    }
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderQuestInfo()
{
    RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y + 160, 188.f, 21.f);

    if (0 == m_CurQuestListBox.GetLineNum() && GameLogic::Quests::KillQuest::GetState().known)
    {
        RenderKillQuest(m_Pos);
    }
    else if (0 == m_CurQuestListBox.GetLineNum())
    {
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetTextColor(255, 255, 0, 255);
        g_pRenderText->SetBgColor(0);
        int i;
        for (i = 0; i < m_nMsgLine; ++i)
            g_pRenderText->RenderText(m_Pos.x + 23, m_Pos.y + 96 + 18 * i,
                m_aszMsg[i], 0, 0, RT3_SORT_LEFT);
    }
    else
        m_CurQuestListBox.Render();

    m_btnQuestOpen.Render();
    m_btnQuestGiveUp.Render();

    m_QuestContentsListBox.Render();
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderJobChangeContents()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(36, 242, 252, 255);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 58, g_csQuest.getQuestTitleWindow(), 190, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0);

    int iY = m_Pos.y + 76;
    for (int i = 0; i < g_iNumLineMessageBoxCustom; ++i)
    {
        g_pRenderText->RenderText(m_Pos.x, iY, g_lpszMessageBoxCustom[i], 190.f, 0.f, RT3_SORT_CENTER);
        iY += 16;
    }
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderJobChangeState()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetBgColor(0);

    BYTE byState = g_csQuest.getCurrQuestState();
    if (byState == QUEST_NONE || byState == QUEST_NO || byState == QUEST_ERROR)
        SetMessage(930);
    else if (byState == QUEST_ING)
        SetMessage(931);
    else if (byState == QUEST_END)
        SetMessage(932);

    int i;
    for (i = 0; i < m_nMsgLine; ++i)
        g_pRenderText->RenderText(m_Pos.x + 23, m_Pos.y + 283 + 18 * i, m_aszMsg[i], 0, 0, RT3_SORT_LEFT);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderDailyInfo()
{
    namespace Illusion = GameLogic::Events::IllusionOfNoria;
    const auto& info = Illusion::GetInfo();
    const float x = m_Pos.x;
    float y = m_Pos.y + 70.f;
    wchar_t text[256];

    const auto line = [&](const wchar_t* value, const BYTE r, const BYTE g, const BYTE b, const bool bold = false)
    {
        g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(r, g, b, 255);
        g_pRenderText->RenderText(x, y, value, 190, 0, RT3_SORT_CENTER);
        y += 16.f;
    };

    line(I18N::Game::IllusionOfNoria, 255, 200, 80, true);
    RenderColorLineARGB(x + 30.f, y - 2.f, x + 160.f, y - 2.f, 1.f, 0xB0C89640u);
    y += 6.f;
    if (!Illusion::HasState())
    {
        line(I18N::Game::LoadingDaily, 160, 160, 160);
        return;
    }

    if (info.QuestState != 2)
    {
        line(I18N::Game::UnlocksAfterTheQuest, 200, 200, 200);
        line(I18N::Game::TheStolenWhistle, 210, 160, 255, true);
        line(I18N::Game::WardenEldrinNoria183102, 200, 200, 200);
        mu_swprintf(text, I18N::Game::ResetsNeededDYouHaveD, info.RequiredResets, info.Resets);
        line(text, info.Resets >= info.RequiredResets ? 120 : 160, info.Resets >= info.RequiredResets ? 255 : 160, info.Resets >= info.RequiredResets ? 120 : 160);
        return;
    }

    line(I18N::Game::DailyQuest, 255, 255, 255, true);
    const int state = Illusion::CurrentDailyState();
    switch (state)
    {
    case 0:
        line(I18N::Game::AvailableTalkToWardenEldrin, 64, 176, 255);
        line(I18N::Game::InTheTownOfTheIllusion, 64, 176, 255);
        break;
    case 1:
    case 2:
        line(state == 2 ? I18N::Game::CompletedClaimYourReward : I18N::Game::InProgress, state == 2 ? 255 : 200, state == 2 ? 200 : 200, state == 2 ? 64 : 200);
        y += 4.f;
        for (const auto& monster : info.Daily)
        {
            const int needed = std::max(1, info.DailyKillsNeeded);
            const int kills = std::min(monster.Kills, needed);
            const float progress = static_cast<float>(kills) / static_cast<float>(needed);
            RenderColorQuadARGB(x + 22.f, y - 1.f, 146.f, 14.f, 0xA0201030u);
            RenderColorQuadARGB(x + 22.f, y - 1.f, 146.f * progress, 14.f, progress >= 1.f ? 0xC0308040u : 0xC07040B0u);
            EnableAlphaTest();
            const wchar_t* name = Illusion::MonsterName(monster.Number);
            mu_swprintf(text, I18N::Game::LsDD, name != nullptr ? name : getMonsterName(monster.Number), kills, needed);
            line(text, 255, 255, 255);
            y += 4.f;
        }

        break;
    default:
    {
        const auto seconds = Illusion::SecondsUntilNextDay();
        mu_swprintf(text, I18N::Game::DailyQuestDoneNextInDHDMin, static_cast<int>(seconds / 3600), static_cast<int>(seconds % 3600 / 60));
        line(text, 160, 160, 160);
        break;
    }
    }

    // the weekly quest: take part in the defeats of the Gilded Colossus
    if (info.WeeklyKillsNeeded > 0)
    {
        y += 8.f;
        line(I18N::Game::WeeklyQuest, 255, 255, 255, true);
        const int weekly = Illusion::CurrentWeeklyState();
        if (weekly == 2)
        {
            const auto seconds = Illusion::SecondsUntilNextWeek();
            mu_swprintf(text, I18N::Game::WeeklyQuestDoneNextInDDDH, static_cast<int>(seconds / 86400), static_cast<int>(seconds % 86400 / 3600));
            line(text, 160, 160, 160);
        }
        else
        {
            if (weekly == 1)
            {
                line(I18N::Game::CompletedClaimYourReward, 255, 200, 64);
            }

            const int needed = std::max(1, info.WeeklyKillsNeeded);
            const int kills = std::min(Illusion::CurrentWeeklyKills(), needed);
            const float progress = static_cast<float>(kills) / static_cast<float>(needed);
            y += 4.f;
            RenderColorQuadARGB(x + 22.f, y - 1.f, 146.f, 14.f, 0xA0201030u);
            RenderColorQuadARGB(x + 22.f, y - 1.f, 146.f * progress, 14.f, progress >= 1.f ? 0xC0308040u : 0xC0B08030u);
            EnableAlphaTest();
            mu_swprintf(text, I18N::Game::LsDD, I18N::Game::GildedColossus, kills, needed);
            line(text, 255, 255, 255);
        }
    }

    y += 8.f;
    mu_swprintf(text, I18N::Game::IllusionShardsU, info.Shards);
    line(text, 210, 160, 255);
}

void SEASON3B::CNewUIMyQuestInfoWindow::OpenningProcess()
{
    g_csQuest.ShowQuestPreviewWindow(-1);
}

void SEASON3B::CNewUIMyQuestInfoWindow::ClosingProcess()
{
    UnselectQuestList();
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    ::PlayBuffer(SOUND_CLICK01);
}

void SEASON3B::CNewUIMyQuestInfoWindow::SetButtonInfo()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_EXIT, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(&I18N::Game::Close388, true);

    m_btnQuestOpen.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_OPEN, false);
    m_btnQuestOpen.ChangeButtonInfo(m_Pos.x + 50, m_Pos.y + 392, 36, 29);
    m_btnQuestOpen.ChangeToolTipText(&I18N::Game::StartQuest, true);

    m_btnQuestGiveUp.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_GIVE_UP, false);
    m_btnQuestGiveUp.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 392, 36, 29);
    m_btnQuestGiveUp.ChangeToolTipText(&I18N::Game::GiveUpQuest, true);
}

CNewUIMyQuestInfoWindow::TAB_BUTTON_INDEX CNewUIMyQuestInfoWindow::UpdateTabBtn()
{
    if (!(SEASON3B::IsPress(VK_LBUTTON)))
        return TAB_NON;

    if (!CheckMouseIn(m_Pos.x + 10, m_Pos.y + 27, 166, 22))
        return TAB_NON;

    if (CheckMouseIn(m_Pos.x + 10, m_Pos.y + 27, 48, 22))
        m_eTabBtnIndex = TAB_QUEST;
    else if (CheckMouseIn(m_Pos.x + 57, m_Pos.y + 27, 48, 22))
        m_eTabBtnIndex = TAB_JOB_CHANGE;
    else if (CheckMouseIn(m_Pos.x + 104, m_Pos.y + 27, 72, 22))
        m_eTabBtnIndex = TAB_DAILY;

    ::PlayBuffer(SOUND_CLICK01);

    return m_eTabBtnIndex;
}

void CNewUIMyQuestInfoWindow::RenderTabBtn()
{
    RenderImage(IMAGE_MYQUEST_TAB_BACK, m_Pos.x + 10, m_Pos.y + 27, 166.f, 22.f);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    if (m_eTabBtnIndex == TAB_QUEST)
    {
        RenderImage(IMAGE_MYQUEST_TAB_SMALL, m_Pos.x + 10, m_Pos.y + 27, 48.f, 22.f);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 34, I18N::Game::Quest, 48, 0, RT3_SORT_CENTER);
        g_pRenderText->SetTextColor(181, 181, 181, 181);
        g_pRenderText->RenderText(m_Pos.x + 57, m_Pos.y + 35, I18N::Game::ChangeClass, 48, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 104, m_Pos.y + 35, I18N::Game::Daily, 72, 0, RT3_SORT_CENTER);
    }
    else if (m_eTabBtnIndex == TAB_JOB_CHANGE)
    {
        RenderImage(IMAGE_MYQUEST_TAB_SMALL, m_Pos.x + 57, m_Pos.y + 27, 48.f, 22.f);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(m_Pos.x + 57, m_Pos.y + 34, I18N::Game::ChangeClass, 48, 0, RT3_SORT_CENTER);
        g_pRenderText->SetTextColor(181, 181, 181, 181);
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 35, I18N::Game::Quest, 48, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 104, m_Pos.y + 35, I18N::Game::Daily, 72, 0, RT3_SORT_CENTER);
    }
    else if (m_eTabBtnIndex == TAB_DAILY)
    {
        RenderImage(IMAGE_MYQUEST_TAB_BIG, m_Pos.x + 104, m_Pos.y + 27, 72.f, 22.f);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(m_Pos.x + 104, m_Pos.y + 34, I18N::Game::Daily, 72, 0, RT3_SORT_CENTER);
        g_pRenderText->SetTextColor(181, 181, 181, 181);
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 35, I18N::Game::Quest, 48, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 57, m_Pos.y + 35, I18N::Game::ChangeClass, 48, 0, RT3_SORT_CENTER);
    }
}

void CNewUIMyQuestInfoWindow::UnselectQuestList()
{
    m_CurQuestListBox.SLSetSelectLine(0);
    m_QuestContentsListBox.Clear();
    QuestOpenBtnEnable(false);
    QuestGiveUpBtnEnable(false);
}

void CNewUIMyQuestInfoWindow::SetCurQuestList(DWordList* pDWordList)
{
    m_CurQuestListBox.Clear();

    wchar_t szInput[64];
    wchar_t szOutput[64];
    g_pRenderText->SetFont(g_hFont);

    int i;
    DWordList::iterator iter;
    for (iter = pDWordList->begin(), i = 1; iter != pDWordList->end(); advance(iter, 1), ++i)
    {
        ::mu_swprintf(szInput, L"%d.%ls", i, g_QuestMng.GetSubject(*iter));
        ::ReduceStringByPixel(szOutput, 64, szInput, 150);
        m_CurQuestListBox.AddText(*iter, szOutput);
    }

    if (m_eTabBtnIndex == TAB_QUEST && 0 == m_CurQuestListBox.GetLineNum())
        SetMessage(2825);

    m_QuestContentsListBox.Clear();
    QuestOpenBtnEnable(false);
    QuestGiveUpBtnEnable(false);
}

void CNewUIMyQuestInfoWindow::SetSelQuestSummary()
{
    m_QuestContentsListBox.Clear();

    DWORD dwSelQuestIndex = GetSelQuestIndex();

    if (0 == dwSelQuestIndex)
        return;

    m_QuestContentsListBox.AddText(
        g_hFontBold, 0xff0ab9ff, RT3_SORT_CENTER, g_QuestMng.GetSubject(dwSelQuestIndex));

    g_pRenderText->SetFont(g_hFont);
    wchar_t aszSummary[8][64];
    int nLine = ::DivideStringByPixel(
        &aszSummary[0][0], 8, 64, g_QuestMng.GetSummary(dwSelQuestIndex), 150);
    int i;
    for (i = 0; i < nLine; ++i)
        m_QuestContentsListBox.AddText(g_hFont, 0xffd2e6ff, RT3_SORT_LEFT, aszSummary[i]);
}

void CNewUIMyQuestInfoWindow::SetSelQuestRequestReward()
{
    DWORD dwSelQuestIndex = GetSelQuestIndex();

    if (0 == dwSelQuestIndex)
        return;

    if (!g_QuestMng.IsRequestRewardQS(dwSelQuestIndex))
        return;

    const SQuestRequestReward* pQuestRequestReward = g_QuestMng.GetRequestReward(dwSelQuestIndex);
    if (NULL == pQuestRequestReward)
        return;

    SRequestRewardText aRequestRewardText[13];
    g_QuestMng.GetRequestRewardText(aRequestRewardText, 13, dwSelQuestIndex);

    int i = 0;
    int j, nLoop;
    for (j = 0; j < 3; ++j)
    {
        if (0 == j)
        {
            m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byRequestCount;
        }
        else if (1 == j && pQuestRequestReward->m_byGeneralRewardCount)
        {
            m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
        }
        else if (2 == j && pQuestRequestReward->m_byRandRewardCount)
        {
            m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
        }
        else
            nLoop = 0;

        for (; i < nLoop; ++i)
            m_QuestContentsListBox.AddText(&aRequestRewardText[i], RT3_SORT_CENTER);
    }
}

void CNewUIMyQuestInfoWindow::QuestOpenBtnEnable(bool bEnable)
{
    if (bEnable)
    {
        m_btnQuestOpen.UnLock();
        m_btnQuestOpen.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    }
    else
    {
        m_btnQuestOpen.Lock();
        m_btnQuestOpen.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
    }
}

void CNewUIMyQuestInfoWindow::QuestGiveUpBtnEnable(bool bEnable)
{
    if (bEnable)
    {
        m_btnQuestGiveUp.UnLock();
        m_btnQuestGiveUp.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    }
    else
    {
        m_btnQuestGiveUp.Lock();
        m_btnQuestGiveUp.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
    }
}

DWORD CNewUIMyQuestInfoWindow::GetSelQuestIndex()
{
    SCurQuestItem* pCurQuestItem = m_CurQuestListBox.GetSelectedText();
    if (NULL == pCurQuestItem)
        return 0;

    return pCurQuestItem->m_dwIndex;
}

void CNewUIMyQuestInfoWindow::SetMessage(int nGlobalTextIndex)
{
    memset(m_aszMsg, 0, sizeof m_aszMsg);
    g_pRenderText->SetFont(g_hFontBold);
    m_nMsgLine = ::DivideStringByPixel(&m_aszMsg[0][0], 2, 64, I18N::Game::Lookup(nGlobalTextIndex), 140);
}
