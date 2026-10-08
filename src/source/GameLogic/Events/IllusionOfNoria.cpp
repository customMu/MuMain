#include "stdafx.h"
#include "GameLogic/Events/IllusionOfNoria.h"

#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "UI/NewUI/Dialogs/IllusionWardenMsgBox.h"

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

        WardenInfo s_info;

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

    const WardenInfo& GetInfo()
    {
        return s_info;
    }

    void ReceiveWardenDialog(const std::span<const std::uint8_t> packet)
    {
        if (packet.size() < HeaderSize)
        {
            return;
        }

        WardenInfo info;
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
            return;
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
            return;
        }

        for (std::size_t i = 0; i < weaponCount; ++i)
        {
            const auto entry = packet.subspan(offset + 1 + (i * WeaponSize), WeaponSize);
            info.Weapons.push_back({ entry[0], entry[1], Read16(entry.subspan(2)), entry[4], entry[5], entry[6], entry[7], entry[8], entry[9], entry[10] != 0 });
        }

        offset += 1 + (weaponCount * WeaponSize);
        if (packet.size() >= offset + 5)
        {
            info.Shards = Read32(packet.subspan(offset));
            const std::size_t exchangeCount = packet[offset + 4];
            if (packet.size() >= offset + 5 + (exchangeCount * ExchangeSize))
            {
                for (std::size_t i = 0; i < exchangeCount; ++i)
                {
                    const auto entry = packet.subspan(offset + 5 + (i * ExchangeSize), ExchangeSize);
                    info.Exchange.push_back({ entry[0], Read16(entry.subspan(1)), Read16(entry.subspan(3)) });
                }
            }
        }

        s_info = std::move(info);
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CIllusionWardenMsgBoxLayout));
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

    void SendBuy(const int group, const int number)
    {
        if (SocketClient == nullptr)
        {
            return;
        }

        const BYTE packet[8] = { C1Header, 8, HeadCode, ActionSubCode, 0x20, static_cast<BYTE>(group),
                                 static_cast<BYTE>(number & 0xFF), static_cast<BYTE>((number >> 8) & 0xFF) };
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

        // the jewels of the harmony options come only from the illusion now and have its names
        wcsncpy_s(ItemAttribute[ITEM_JEWEL_OF_HARMONY].Name, I18N::Game::JewelOfIllusion, _TRUNCATE);
        wcsncpy_s(ItemAttribute[ITEM_LOWER_REFINE_STONE].Name, I18N::Game::LesserMirageStone, _TRUNCATE);
        wcsncpy_s(ItemAttribute[ITEM_HIGHER_REFINE_STONE].Name, I18N::Game::GreaterMirageStone, _TRUNCATE);
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
