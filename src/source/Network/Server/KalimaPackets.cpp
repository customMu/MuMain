#include "stdafx.h"
#include "Network/Server/KalimaPackets.h"

#include <vector>

#include "GameLogic/Events/KalimaEntry.h"
#include "GameLogic/Items/KundunEssence.h"
#include "GameLogic/Travel/MinimapSpots.h"
#include "Network/Server/WSclient.h"
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
        constexpr std::size_t SpotHeaderSize = 6;
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
                const std::size_t nameLength = packet[offset + 5];
                if (offset + SpotHeaderSize + nameLength > packet.size())
                {
                    break;
                }

                GameLogic::Travel::MinimapSpots::Spot spot{ packet[offset], packet[offset + 1], ReadUInt16LittleEndian(packet.subspan(offset + 2)), packet[offset + 4], {} };
                const auto* name = reinterpret_cast<const char*>(packet.data() + offset + SpotHeaderSize);
                const int wideLength = MultiByteToWideChar(CP_UTF8, 0, name, static_cast<int>(nameLength), nullptr, 0);
                spot.name.resize(static_cast<std::size_t>(wideLength));
                MultiByteToWideChar(CP_UTF8, 0, name, static_cast<int>(nameLength), spot.name.data(), wideLength);
                spots.push_back(std::move(spot));
                offset += SpotHeaderSize + nameLength;
            }

            GameLogic::Travel::MinimapSpots::SetSpots(std::move(spots));
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
