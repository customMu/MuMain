#include "stdafx.h"
#include "GameLogic/Events/IllusionOfNoria.h"

#include "Engine/Object/ZzzInfomation.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Combat/SkillCastTimeOptions.h"
#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "UI/NewUI/Dialogs/IllusionWardenMsgBox.h"
#include "Camera/CameraProjection.h"
#include "Camera/CameraState.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/SceneCore.h"
#include "World/MapInfra/MapManager.h"

namespace GameLogic::Events::IllusionOfNoria
{
    namespace
    {
        constexpr std::uint8_t C1Header = 0xC1;
        constexpr std::uint8_t HeadCode = 0xFB;
        constexpr std::uint8_t ActionSubCode = 0x13;
        constexpr std::size_t HeaderSize = 20;
        constexpr std::size_t DailySize = 4;
        constexpr std::size_t WeaponSize = 11;
        constexpr std::size_t ExchangeSize = 5;
        constexpr std::size_t WeeklySize = 10;
        constexpr std::size_t WeeklyRewardSize = 5;

        WardenInfo s_info;
        bool s_hasState = false;
        std::uint64_t s_receivedAt = 0; // GetTickCount64 of the last state
        int s_lastWorld = -1;
        std::uint64_t s_curseEndsAt = 0; // GetTickCount64
        std::uint64_t s_wardEndsAt = 0;  // GetTickCount64
        std::uint64_t s_blessingEndsAt = 0; // GetTickCount64

        // the mark: a bar and a dot with a dark outline, bobbing over the head
        void DrawMark(const float x, const float y, const unsigned int color)
        {
            constexpr unsigned int Outline = 0xE0101018u;
            RenderColorQuadARGB(x - 6.f, y - 4.f, 12.f, 26.f, 0x30000000u | (color & 0x00FFFFFFu)); // a soft glow
            RenderColorQuadARGB(x - 3.f, y - 1.f, 6.f, 15.f, Outline);
            RenderColorQuadARGB(x - 3.f, y + 15.f, 6.f, 6.f, Outline);
            RenderColorQuadARGB(x - 2.f, y, 4.f, 13.f, color);
            RenderColorQuadARGB(x - 2.f, y + 16.f, 4.f, 4.f, color);
        }

        bool Parse(const std::span<const std::uint8_t> packet, WardenInfo& info);

        std::uint16_t Read16(std::span<const std::uint8_t> data)
        {
            return static_cast<std::uint16_t>(data[0] | (data[1] << 8));
        }

        std::uint32_t Read32(std::span<const std::uint8_t> data)
        {
            return static_cast<std::uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (static_cast<std::uint32_t>(data[3]) << 24));
        }
    }

    const Echo* FindEcho(const int itemType)
    {
        for (const auto& echo : Echoes)
        {
            if (echo.Item == itemType)
            {
                return &echo;
            }
        }

        return nullptr;
    }

    bool IsSkillFixStone(const int itemType)
    {
        return itemType == ITEM_JEWEL_OF_ILLUSION || itemType == ITEM_LESSER_MIRAGE_STONE || itemType == ITEM_GREATER_MIRAGE_STONE
            || FindEcho(itemType) != nullptr;
    }

    bool CanApplyStone(const int stoneType, const ITEM& target)
    {
        // only the Mirage Stones are used from the inventory; the Jewel of Illusion and the Echoes add the option in the
        // Chaos Machine of the Illusion of Noria (mix 90, server IllusionAddSkillFixCrafting, 09.10.2026)
        namespace CastTime = GameLogic::Combat::SkillCastTime;
        if (stoneType == ITEM_LESSER_MIRAGE_STONE || stoneType == ITEM_GREATER_MIRAGE_STONE)
        {
            return target.SkillFixOption != 0 && target.SkillFixLevel < CastTime::MaxOptionLevel;
        }

        return false;
    }

    const WardenInfo& GetInfo()
    {
        return s_info;
    }

    void ReceiveWardenDialog(const std::span<const std::uint8_t> packet)
    {
        WardenInfo info;
        if (!Parse(packet, info))
        {
            return;
        }

        s_info = std::move(info);
        s_hasState = true;
        s_receivedAt = GetTickCount64();
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CIllusionWardenMsgBoxLayout));
    }

    void ReceiveWardenState(const std::span<const std::uint8_t> packet)
    {
        WardenInfo info;
        if (!Parse(packet, info))
        {
            return;
        }

        s_info = std::move(info);
        s_hasState = true;
        s_receivedAt = GetTickCount64();
    }

    void ReceiveCurseTime(const int seconds)
    {
        s_curseEndsAt = GetTickCount64() + static_cast<std::uint64_t>(std::max(0, seconds)) * 1000;
    }

    void FormatOption(wchar_t* out, const size_t size, const int option, const int level, const int family, const wchar_t* skillName)
    {
        namespace CastTime = GameLogic::Combat::SkillCastTime;
        wchar_t value[16];
        mu_swprintf_s(value, std::size(value), L"%.2f%%", CastTime::ExtraOptionValue(option, level) * 100.f);
        switch (CastTime::EffectOf(option))
        {
        case CastTime::IllusionEffect::Haste:
            mu_swprintf_s(out, size, I18N::Game::HasteOfLsD, skillName, static_cast<int>(CastTime::OptionCutOf(level, family) * 100.f + 0.5f));
            break;
        case CastTime::IllusionEffect::Vampiric:
            mu_swprintf_s(out, size, I18N::Game::VampiricLsLs, skillName, value);
            break;
        case CastTime::IllusionEffect::Siphon:
            mu_swprintf_s(out, size, I18N::Game::SiphonLsLs, skillName, value);
            break;
        case CastTime::IllusionEffect::Fury:
            mu_swprintf_s(out, size, I18N::Game::FuryOfLsLs, skillName, value);
            break;
        default:
            if (size > 0)
            {
                out[0] = L'\0';
            }

            break;
        }
    }

    int CurseSecondsLeft()
    {
        const auto now = GetTickCount64();
        return now >= s_curseEndsAt ? 0 : static_cast<int>((s_curseEndsAt - now + 999) / 1000);
    }

    void ReceiveWardTime(const int seconds)
    {
        s_wardEndsAt = GetTickCount64() + static_cast<std::uint64_t>(std::max(0, seconds)) * 1000;
    }

    int WardSecondsLeft()
    {
        const auto now = GetTickCount64();
        return now >= s_wardEndsAt ? 0 : static_cast<int>((s_wardEndsAt - now + 999) / 1000);
    }

    void ReceiveBlessingTime(const int seconds)
    {
        s_blessingEndsAt = GetTickCount64() + static_cast<std::uint64_t>(std::max(0, seconds)) * 1000;
    }

    int BlessingSecondsLeft()
    {
        const auto now = GetTickCount64();
        return now >= s_blessingEndsAt ? 0 : static_cast<int>((s_blessingEndsAt - now + 999) / 1000);
    }

    bool HasState()
    {
        return s_hasState;
    }

    std::uint32_t SecondsUntilNextDay()
    {
        const auto elapsed = static_cast<std::uint32_t>((GetTickCount64() - s_receivedAt) / 1000);
        return elapsed >= s_info.SecondsUntilNextDay ? 0 : s_info.SecondsUntilNextDay - elapsed;
    }

    std::uint32_t SecondsUntilNextWeek()
    {
        const auto elapsed = static_cast<std::uint32_t>((GetTickCount64() - s_receivedAt) / 1000);
        return elapsed >= s_info.SecondsUntilNextWeek ? 0 : s_info.SecondsUntilNextWeek - elapsed;
    }

    int CurrentWeeklyState()
    {
        // a new week: the weekly quest done this week starts again
        if (s_info.WeeklyState == 2 && SecondsUntilNextWeek() == 0)
        {
            return 0;
        }

        return s_info.WeeklyState;
    }

    int CurrentWeeklyKills()
    {
        return s_info.WeeklyState != 1 && SecondsUntilNextWeek() == 0 ? 0 : s_info.WeeklyKills;
    }

    int CurrentDailyState()
    {
        // a new day: the daily quest done today (or not claimed) can be taken again
        if (s_info.DailyState != 0 && s_info.DailyState != 1 && SecondsUntilNextDay() == 0)
        {
            return 0;
        }

        return s_info.DailyState;
    }

    int WardenMark()
    {
        if (!s_hasState)
        {
            return 0;
        }

        if (gMapManager.IsIllusionOfNoria())
        {
            if (s_info.QuestState != 2)
            {
                return 0;
            }

            if (CurrentWeeklyState() == 1)
            {
                return 2;
            }

            switch (CurrentDailyState())
            {
            case 0: return 1;
            case 2: return 2;
            default: return 0;
            }
        }

        if (s_info.QuestState == 0 && s_info.Resets >= s_info.RequiredResets)
        {
            return 1;
        }

        return s_info.QuestState == 1 && s_info.HasWhistle ? 2 : 0;
    }

    void RenderWardenMark()
    {
        if (Hero == nullptr || SceneFlag != MAIN_SCENE)
        {
            return;
        }

        if (s_lastWorld != gMapManager.WorldActive)
        {
            s_lastWorld = gMapManager.WorldActive;
            SendAction(State);
        }

        if (!gMapManager.IsNoriaWorld())
        {
            return;
        }

        const int mark = WardenMark();
        if (mark == 0)
        {
            return;
        }

        for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
        {
            CHARACTER* c = &CharactersClient[i];
            OBJECT* o = &c->Object;
            if (!o->Live || !o->Visible || o->Kind != KIND_NPC || c->MonsterIndex != Warden)
            {
                continue;
            }

            vec3_t position;
            Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 90.f, position);
            vec3_t transformed;
            VectorTransform(position, g_Camera.Matrix, transformed);
            if (transformed[2] >= 0)
            {
                continue;
            }

            int screenX = 0;
            int screenY = 0;
            CameraProjection::WorldToScreen(g_Camera, position, &screenX, &screenY);
            const float bob = sinf(static_cast<float>(WorldTime) * 0.004f) * 3.f;
            DrawMark(static_cast<float>(screenX), static_cast<float>(screenY) - 24.f + bob, mark == 1 ? 0xFF40B0FFu : 0xFFFFC840u);
        }
    }

    namespace
    {
    bool Parse(const std::span<const std::uint8_t> packet, WardenInfo& info)
    {
        if (packet.size() < HeaderSize)
        {
            return false;
        }

        info.InIllusion = packet[5] != 0;
        info.QuestState = packet[6];
        info.HasWhistle = packet[7] != 0;
        info.Resets = Read16(packet.subspan(8));
        info.RequiredResets = Read16(packet.subspan(10));
        info.DailyState = packet[12];
        const std::size_t dailyCount = packet[13];
        info.DailyKillsNeeded = Read16(packet.subspan(14));
        info.SecondsUntilNextDay = Read32(packet.subspan(16));
        std::size_t offset = HeaderSize;
        if (packet.size() < offset + (dailyCount * DailySize) + 1)
        {
            return false;
        }

        for (std::size_t i = 0; i < dailyCount; ++i)
        {
            const auto entry = packet.subspan(offset + (i * DailySize), DailySize);
            info.Daily.push_back({ Read16(entry), Read16(entry.subspan(2)) });
        }

        offset += dailyCount * DailySize;
        const std::size_t weaponCount = packet[offset];
        if (packet.size() < offset + 1 + (weaponCount * WeaponSize))
        {
            return false;
        }

        for (std::size_t i = 0; i < weaponCount; ++i)
        {
            const auto entry = packet.subspan(offset + 1 + (i * WeaponSize), WeaponSize);
            info.Weapons.push_back({ entry[0], entry[1], Read16(entry.subspan(2)), entry[4], entry[5], entry[6], entry[7], entry[8], entry[9], entry[10] != 0 });
        }

        offset += 1 + (weaponCount * WeaponSize);
        if (packet.size() < offset + 5)
        {
            return true;
        }

        info.Shards = Read32(packet.subspan(offset));
        const std::size_t exchangeCount = packet[offset + 4];
        if (packet.size() < offset + 5 + (exchangeCount * ExchangeSize))
        {
            return true;
        }

        for (std::size_t i = 0; i < exchangeCount; ++i)
        {
            const auto entry = packet.subspan(offset + 5 + (i * ExchangeSize), ExchangeSize);
            info.Exchange.push_back({ entry[0], Read16(entry.subspan(1)), Read16(entry.subspan(3)) });
        }

        // the weekly quest (servers since 10.10.2026)
        offset += 5 + (exchangeCount * ExchangeSize);
        if (packet.size() < offset + WeeklySize)
        {
            return true;
        }

        info.WeeklyState = packet[offset];
        info.WeeklyKills = packet[offset + 1];
        info.WeeklyKillsNeeded = packet[offset + 2];
        info.WeeklyMinimumDamageTenths = Read16(packet.subspan(offset + 3));
        info.SecondsUntilNextWeek = Read32(packet.subspan(offset + 5));
        const std::size_t rewardCount = packet[offset + 9];
        if (packet.size() >= offset + WeeklySize + (rewardCount * WeeklyRewardSize))
        {
            for (std::size_t i = 0; i < rewardCount; ++i)
            {
                const auto entry = packet.subspan(offset + WeeklySize + (i * WeeklyRewardSize), WeeklyRewardSize);
                info.WeeklyRewards.push_back({ entry[0], Read16(entry.subspan(1)), Read16(entry.subspan(3)) });
            }
        }

        return true;
    }
    }

    void SendAction(const std::uint8_t action)
    {
        if (SocketClient == nullptr)
        {
            return;
        }

        const BYTE packet[5] = { C1Header, 5, HeadCode, ActionSubCode, action };
        SocketClient->Send(packet, static_cast<int32_t>(sizeof packet));
    }

    void RegisterItems()
    {
        // the items are not in Item_<lang>.bmd: they take the attributes of a similar item and their own name and size
        ITEM_ATTRIBUTE& whistle = ItemAttribute[ITEM_WHISTLE_OF_THE_VEIL];
        whistle = ItemAttribute[ITEM_LOST_MAP];
        wcsncpy_s(whistle.Name, I18N::Game::WhistleOfTheVeil, _TRUNCATE);
        whistle.Width = 1;
        whistle.Height = 2;

        ITEM_ATTRIBUTE& shard = ItemAttribute[ITEM_ILLUSION_SHARD];
        shard = ItemAttribute[ITEM_JEWEL_OF_HARMONY];
        wcsncpy_s(shard.Name, I18N::Game::IllusionShard, _TRUNCATE);

        for (const auto& echo : Echoes)
        {
            ITEM_ATTRIBUTE& attribute = ItemAttribute[echo.Item];
            attribute = ItemAttribute[ITEM_JEWEL_OF_HARMONY];
            mu_swprintf(attribute.Name, I18N::Game::EchoOfLs, echo.SkillName);
        }

        // the stones of the skill fix option (not the harmony jewels, which stay as they are)
        const std::pair<int, const wchar_t*> stones[] = {
            { ITEM_JEWEL_OF_ILLUSION, I18N::Game::JewelOfIllusion },
            { ITEM_LESSER_MIRAGE_STONE, I18N::Game::LesserMirageStone },
            { ITEM_GREATER_MIRAGE_STONE, I18N::Game::GreaterMirageStone },
        };
        for (const auto& [type, name] : stones)
        {
            ITEM_ATTRIBUTE& attribute = ItemAttribute[type];
            attribute = ItemAttribute[ITEM_JEWEL_OF_HARMONY];
            wcsncpy_s(attribute.Name, name, _TRUNCATE);
        }
    }

    const wchar_t* MonsterName(const int type)
    {
        switch (type)
        {
        case MONSTER_ILLUSION_GOBLIN: return I18N::Game::IllusionGoblin;
        case MONSTER_ILLUSION_CHAIN_SCORPION: return I18N::Game::IllusionChainScorpion;
        case MONSTER_ILLUSION_BEETLE_MONSTER: return I18N::Game::IllusionBeetleMonster;
        case MONSTER_ILLUSION_HUNTER: return I18N::Game::IllusionHunter;
        case MONSTER_ILLUSION_FOREST_MONSTER: return I18N::Game::IllusionForestMonster;
        case MONSTER_ILLUSION_AGON: return I18N::Game::IllusionAgon;
        case MONSTER_ILLUSION_STONE_GOLEM: return I18N::Game::IllusionStoneGolem;
        case MONSTER_ILLUSION_ELITE_GOBLIN: return I18N::Game::IllusionEliteGoblin;
        case MONSTER_GILDED_COLOSSUS: return I18N::Game::GildedColossus;
        case MONSTER_WARDEN_ELDRIN: return I18N::Game::WardenEldrin;
        default: return nullptr;
        }
    }
}
