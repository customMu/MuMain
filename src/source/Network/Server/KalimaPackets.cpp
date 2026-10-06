#include "stdafx.h"
#include "GameLogic/Quests/KillQuest.h"
#include "Data/Translation/MultiLanguage.h"
#include "Network/Server/KalimaPackets.h"

#include <vector>

#include "GameLogic/Events/KalimaEntry.h"
#include "GameLogic/Events/KalimaSpots.h"
#include "GameLogic/Events/KundunChamberEntry.h"
#include "GameLogic/Items/KundunEssence.h"
#include "GameLogic/Travel/MinimapSpots.h"
#include "Network/Server/WSclient.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"

namespace Network::Server::KalimaPackets
{
    namespace
    {
        constexpr std::uint8_t BalanceSubCode = 0x01;
        constexpr std::uint8_t ShopPricesSubCode = 0x02;
        constexpr std::uint8_t DropModeSubCode = 0x03;
        constexpr std::uint8_t KalimaEntrySubCode = 0x04;
        constexpr std::uint8_t KalimaEnterSubCode = 0x05;
        constexpr std::uint8_t MinimapSpotsSubCode = 0x06;
        constexpr std::size_t SpotsCountOffset = 5;
        constexpr std::size_t SpotsEntriesOffset = 7;
        constexpr std::size_t SpotHeaderSize = 7;   // x, y, level (u16), monsters, flags (1 = boss), name length
        constexpr std::uint8_t KalimaSpotsSubCode = 0x07;
        constexpr std::size_t SpotCountOffset = 4;
        constexpr std::size_t SpotEntriesOffset = 5;
        constexpr std::size_t SpotEntrySize = 2;
        constexpr std::uint8_t KalimaProgressSubCode = 0x08;
        constexpr std::size_t KalimaProgressPacketSize = 7;
        constexpr std::uint8_t KalimaArenaSubCode = 0x09;
        constexpr std::uint8_t ChamberEntrySubCode = 0x0A;
        constexpr std::uint8_t ChamberEnterSubCode = 0x0B;
        constexpr std::uint8_t ChamberStatusSubCode = 0x0C;
        constexpr std::uint8_t KillQuestStateSubCode = 0x0D;
        constexpr std::uint8_t KillQuestClaimSubCode = 0x0E;
        constexpr std::uint8_t DropModeVoteSubCode = 0x0F;
        constexpr std::uint8_t DropModeVoteAnswerSubCode = 0x10;
        constexpr std::uint8_t InviteDropModeSubCode = 0x11;
        constexpr std::size_t DropModeVotePacketSize = 15;
        constexpr std::size_t DropModeVoteNameOffset = 5;
        constexpr std::size_t DropModeVoteNameLength = 10;
        constexpr std::size_t KillQuestStatePacketSize = 117;
        constexpr std::size_t ChamberStatusPacketSize = 15;
        constexpr std::size_t ChamberHeaderSize = 18;
        constexpr std::size_t KalimaArenaPacketSize = 7;
        constexpr std::size_t KalimaHeaderSize = 17;
        constexpr std::size_t KalimaTierSize = 5;

        constexpr std::uint8_t C1Header = 0xC1;
        constexpr std::size_t C1SubCodeOffset = 3;
        constexpr std::size_t C2SubCodeOffset = 4;
        constexpr std::size_t BalanceOffset = 4;
        constexpr std::size_t BalancePacketSize = 8;
        constexpr std::size_t PriceCountOffset = 5;
        constexpr std::size_t PriceEntriesOffset = 6;
        constexpr std::size_t PriceEntrySize = 5;
        constexpr std::size_t DropModeOffset = 4;
        constexpr std::size_t DropModePacketSize = 5;

        std::uint32_t ReadUInt32LittleEndian(const std::span<const std::uint8_t> data)
        {
            return static_cast<std::uint32_t>(data[0])
                | (static_cast<std::uint32_t>(data[1]) << 8)
                | (static_cast<std::uint32_t>(data[2]) << 16)
                | (static_cast<std::uint32_t>(data[3]) << 24);
        }

        void ReceiveBalance(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < BalancePacketSize)
            {
                return;
            }

            GameLogic::Items::KundunEssence::SetBalance(ReadUInt32LittleEndian(packet.subspan(BalanceOffset)));
        }

        void ReceiveShopPrices(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() <= PriceCountOffset)
            {
                return;
            }

            const std::size_t count = packet[PriceCountOffset];
            if (packet.size() < PriceEntriesOffset + (count * PriceEntrySize))
            {
                return;
            }

            std::vector<GameLogic::Items::KundunEssence::ShopPrice> prices;
            prices.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
            {
                const auto entry = packet.subspan(PriceEntriesOffset + (i * PriceEntrySize), PriceEntrySize);
                prices.push_back({ entry[0], ReadUInt32LittleEndian(entry.subspan(1)) });
            }

            GameLogic::Items::KundunEssence::SetShopPrices(prices);
        }

        std::uint16_t ReadUInt16LittleEndian(const std::span<const std::uint8_t> data)
        {
            return static_cast<std::uint16_t>(data[0] | (data[1] << 8));
        }

        void ReceiveKalimaEntry(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < KalimaHeaderSize)
            {
                return;
            }

            GameLogic::Events::KalimaEntry::Info info;
            info.resets = ReadUInt16LittleEndian(packet.subspan(5));
            info.tierLevel = packet[7];
            info.entriesLeft = packet[8];
            info.entriesPerDay = packet[9];
            info.secondsUntilReset = ReadUInt32LittleEndian(packet.subspan(10));
            info.canReenter = packet[14] != 0;
            const std::size_t count = packet[15];
            if (packet.size() < KalimaHeaderSize + (count * KalimaTierSize))
            {
                return;
            }

            for (std::size_t i = 0; i < count; ++i)
            {
                const auto entry = packet.subspan(KalimaHeaderSize + (i * KalimaTierSize), KalimaTierSize);
                info.tiers.push_back({ entry[0], ReadUInt16LittleEndian(entry.subspan(1)), ReadUInt16LittleEndian(entry.subspan(3)) });
            }

            GameLogic::Events::KalimaEntry::SetInfo(std::move(info));
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CKalimaEntryMsgBoxLayout));
        }

        void ReceiveMinimapSpots(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < SpotsEntriesOffset)
            {
                return;
            }

            const std::size_t count = ReadUInt16LittleEndian(packet.subspan(SpotsCountOffset));
            std::vector<GameLogic::Travel::MinimapSpots::Spot> spots;
            spots.reserve(count);
            std::size_t offset = SpotsEntriesOffset;
            for (std::size_t i = 0; i < count && offset + SpotHeaderSize <= packet.size(); ++i)
            {
                const std::size_t nameLength = packet[offset + 6];
                if (offset + SpotHeaderSize + nameLength > packet.size())
                {
                    break;
                }

                GameLogic::Travel::MinimapSpots::Spot spot{ packet[offset], packet[offset + 1], ReadUInt16LittleEndian(packet.subspan(offset + 2)), packet[offset + 4], {} };
                spot.boss = (packet[offset + 5] & 1) != 0;
                const auto* name = reinterpret_cast<const char*>(packet.data() + offset + SpotHeaderSize);
                const int wideLength = MultiByteToWideChar(CP_UTF8, 0, name, static_cast<int>(nameLength), nullptr, 0);
                spot.name.resize(static_cast<std::size_t>(wideLength));
                MultiByteToWideChar(CP_UTF8, 0, name, static_cast<int>(nameLength), spot.name.data(), wideLength);
                spots.push_back(std::move(spot));
                offset += SpotHeaderSize + nameLength;
            }

            GameLogic::Travel::MinimapSpots::SetSpots(std::move(spots));
        }

        void ReceiveKalimaSpots(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() <= SpotCountOffset)
            {
                return;
            }

            const std::size_t count = packet[SpotCountOffset];
            if (packet.size() < SpotEntriesOffset + (count * SpotEntrySize))
            {
                return;
            }

            std::vector<GameLogic::Events::KalimaSpots::Spot> spots;
            spots.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
            {
                const auto entry = packet.subspan(SpotEntriesOffset + (i * SpotEntrySize), SpotEntrySize);
                spots.push_back({ entry[0], entry[1] });
            }

            GameLogic::Events::KalimaSpots::SetSpots(std::move(spots));
        }

        void ReceiveKalimaProgress(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < KalimaProgressPacketSize)
            {
                return;
            }

            GameLogic::Events::KalimaSpots::SetProgress({ packet[4], packet[5], static_cast<GameLogic::Events::KalimaSpots::BossState>(packet[6]) });
        }

        std::wstring ReadUtf8(const std::span<const std::uint8_t> bytes)
        {
            std::size_t length = 0;
            while (length < bytes.size() && bytes[length] != 0)
            {
                ++length;
            }

            const std::string text(reinterpret_cast<const char*>(bytes.data()), length);
            wchar_t wide[128] = {};
            CMultiLanguage::ConvertFromUtf8(wide, text.c_str(), static_cast<int>(text.size()));
            return wide;
        }

        // The kill quests: [quest number u16] [quest count u16] [kills u32] [needed u32] [reward points u16] [waiting]
        // [stat points from quests u16] [monster, 32 bytes UTF-8] [reward items, 64 bytes UTF-8]
        void ReceiveKillQuestState(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < KillQuestStatePacketSize)
            {
                return;
            }

            const auto u16 = [&](std::size_t offset) { return static_cast<int>(packet[offset] | (packet[offset + 1] << 8)); };
            const auto u32 = [&](std::size_t offset) { return static_cast<int>(packet[offset] | (packet[offset + 1] << 8) | (packet[offset + 2] << 16) | (packet[offset + 3] << 24)); };
            GameLogic::Quests::KillQuest::State state;
            state.number = u16(4);
            state.count = u16(6);
            state.kills = u32(8);
            state.killsNeeded = u32(12);
            state.rewardPoints = u16(16);
            state.rewardWaiting = packet[18] != 0;
            state.questPoints = u16(19);
            state.monster = ReadUtf8(packet.subspan(21, 32));
            state.reward = ReadUtf8(packet.subspan(53, 64));
            GameLogic::Quests::KillQuest::SetState(std::move(state));
        }

        // A member proposes a new drop mode: the hero agrees or not (message box).
        void ReceiveDropModeVote(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < DropModeVotePacketSize)
            {
                return;
            }

            GameLogic::Social::PartyDropMode::SetVote(
                static_cast<GameLogic::Social::PartyDropMode::Mode>(packet[DropModeOffset]),
                ReadUtf8(packet.subspan(DropModeVoteNameOffset, DropModeVoteNameLength)));
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPartyDropModeVoteMsgBoxLayout));
        }

        void ReceiveChamberStatus(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < ChamberStatusPacketSize)
            {
                return;
            }

            GameLogic::Events::KalimaSpots::ChamberStatus status;
            status.level = packet[4];
            status.phase = packet[5];
            status.phaseCount = packet[6];
            status.healthPercent = packet[7];
            status.illusions = packet[8];
            status.shielded = (packet[9] & 1) != 0;
            status.defeated = (packet[9] & 2) != 0;
            status.secondsLeft = static_cast<std::uint16_t>(packet[10] | (packet[11] << 8));
            status.receivedAt = GetTickCount64();
            status.lastHealPercent = packet[12];
            status.lastDefensePercent = packet[13];
            status.lastDamagePercent = packet[14];
            GameLogic::Events::KalimaSpots::SetChamberStatus(status);
        }

        // The chamber of Kundun: everything outside of the ring of columns is not walkable, until the map is loaded again.
        void ReceiveKalimaArena(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < KalimaArenaPacketSize)
            {
                return;
            }

            const int centerX = packet[4];
            const int centerY = packet[5];
            const float radius = packet[6] / 2.f;
            for (int y = 0; y < TERRAIN_SIZE; ++y)
            {
                for (int x = 0; x < TERRAIN_SIZE; ++x)
                {
                    const float dx = static_cast<float>(x - centerX);
                    const float dy = static_cast<float>(y - centerY);
                    if (std::sqrt((dx * dx) + (dy * dy)) >= radius)
                    {
                        TerrainWall[(y * TERRAIN_SIZE) + x] |= TW_NOMOVE;
                    }
                }
            }

            GameLogic::Events::KalimaSpots::SetArenaClosed(true);
        }

        void ReceiveChamberEntry(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < ChamberHeaderSize)
            {
                return;
            }

            GameLogic::Events::KundunChamberEntry::Info info;
            info.resets = ReadUInt16LittleEndian(packet.subspan(5));
            info.tierLevel = packet[7];
            info.entriesLeft = packet[8];
            info.entriesPerWeek = packet[9];
            info.secondsUntilReset = ReadUInt32LittleEndian(packet.subspan(10));
            info.canReenter = packet[14] != 0;
            info.hasLostMap = packet[15] != 0;
            const std::size_t count = packet[16];
            if (packet.size() < ChamberHeaderSize + (count * KalimaTierSize))
            {
                return;
            }

            for (std::size_t i = 0; i < count; ++i)
            {
                const auto entry = packet.subspan(ChamberHeaderSize + (i * KalimaTierSize), KalimaTierSize);
                info.tiers.push_back({ entry[0], ReadUInt16LittleEndian(entry.subspan(1)), ReadUInt16LittleEndian(entry.subspan(3)) });
            }

            GameLogic::Events::KundunChamberEntry::SetInfo(std::move(info));
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CKundunChamberEntryMsgBoxLayout));
        }

        void ReceiveDropMode(const std::span<const std::uint8_t> packet)
        {
            if (packet.size() < DropModePacketSize)
            {
                return;
            }

            GameLogic::Social::PartyDropMode::SetMode(static_cast<GameLogic::Social::PartyDropMode::Mode>(packet[DropModeOffset]));
        }
    }

    void ReceivePacket(const std::span<const std::uint8_t> packet)
    {
        const bool isC1 = !packet.empty() && packet[0] == C1Header;
        const std::size_t subCodeOffset = isC1 ? C1SubCodeOffset : C2SubCodeOffset;
        if (packet.size() <= subCodeOffset)
        {
            return;
        }

        switch (packet[subCodeOffset])
        {
        case BalanceSubCode:
            ReceiveBalance(packet);
            break;
        case ShopPricesSubCode:
            ReceiveShopPrices(packet);
            break;
        case DropModeSubCode:
            if (isC1)
            {
                ReceiveDropMode(packet);
            }

            break;
        case DropModeVoteSubCode:
            if (isC1)
            {
                ReceiveDropModeVote(packet);
            }

            break;
        case InviteDropModeSubCode:
            if (isC1 && packet.size() >= DropModePacketSize)
            {
                GameLogic::Social::PartyDropMode::SetInviteMode(static_cast<GameLogic::Social::PartyDropMode::Mode>(packet[DropModeOffset]));
            }

            break;
        case KalimaEntrySubCode:
            if (!isC1)
            {
                ReceiveKalimaEntry(packet);
            }

            break;
        case MinimapSpotsSubCode:
            if (!isC1)
            {
                ReceiveMinimapSpots(packet);
            }

            break;
        case KalimaSpotsSubCode:
            if (isC1)
            {
                ReceiveKalimaSpots(packet);
            }

            break;
        case KalimaProgressSubCode:
            if (isC1)
            {
                ReceiveKalimaProgress(packet);
            }

            break;
        case KillQuestStateSubCode:
            if (isC1)
            {
                ReceiveKillQuestState(packet);
            }

            break;
        case ChamberStatusSubCode:
            if (isC1)
            {
                ReceiveChamberStatus(packet);
            }

            break;
        case KalimaArenaSubCode:
            if (isC1)
            {
                ReceiveKalimaArena(packet);
            }

            break;
        case ChamberEntrySubCode:
            if (!isC1)
            {
                ReceiveChamberEntry(packet);
            }

            break;
        default:
            break;
        }
    }

    void SendPartyDropMode(const GameLogic::Social::PartyDropMode::Mode mode)
    {
        if (SocketClient == nullptr)
        {
            return;
        }

        const BYTE packet[DropModePacketSize] = { C1Header, static_cast<BYTE>(DropModePacketSize), HeadCode, DropModeSubCode, static_cast<BYTE>(mode) };
        SocketClient->Send(packet, static_cast<int32_t>(sizeof packet));
    }

    void SendPartyDropModeVoteAnswer(const bool accept)
    {
        if (SocketClient == nullptr)
        {
            return;
        }

        const BYTE packet[DropModePacketSize] = { C1Header, static_cast<BYTE>(DropModePacketSize), HeadCode, DropModeVoteAnswerSubCode, static_cast<BYTE>(accept ? 1 : 0) };
        SocketClient->Send(packet, static_cast<int32_t>(sizeof packet));
    }

    void SendChamberEnterRequest()
    {
        if (SocketClient == nullptr)
        {
            return;
        }

        const BYTE packet[4] = { C1Header, 4, HeadCode, ChamberEnterSubCode };
        SocketClient->Send(packet, static_cast<int32_t>(sizeof packet));
    }

    void SendKillQuestRewardRequest()
    {
        if (SocketClient == nullptr)
        {
            return;
        }

        const BYTE packet[4] = { C1Header, 4, HeadCode, KillQuestClaimSubCode };
        SocketClient->Send(packet, static_cast<int32_t>(sizeof packet));
    }

    void SendKalimaEnterRequest()
    {
        if (SocketClient == nullptr)
        {
            return;
        }

        const BYTE packet[4] = { C1Header, 4, HeadCode, KalimaEnterSubCode };
        SocketClient->Send(packet, static_cast<int32_t>(sizeof packet));
    }
}
