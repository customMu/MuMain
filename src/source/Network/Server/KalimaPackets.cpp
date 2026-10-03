#include "stdafx.h"
#include "Network/Server/KalimaPackets.h"

#include <vector>

#include "GameLogic/Items/KundunEssence.h"
#include "Network/Server/WSclient.h"

namespace Network::Server::KalimaPackets
{
    namespace
    {
        constexpr std::uint8_t BalanceSubCode = 0x01;
        constexpr std::uint8_t ShopPricesSubCode = 0x02;
        constexpr std::uint8_t DropModeSubCode = 0x03;

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
}
