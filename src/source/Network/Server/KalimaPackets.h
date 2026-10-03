#pragma once

#include <cstdint>
#include <span>

#include "GameLogic/Social/PartyDropMode.h"

namespace Network::Server::KalimaPackets
{
    // Custom packets of this server (OpenMU: KundunEssenceViewPlugIn, PartyDropModeViewPlugIn), head code 0xFB:
    //   C1 08 FB 01 [balance: uint32 little endian]                         - Kundun Essence balance
    //   C2 [size: uint16 big endian] FB 02 [count] count x [slot, price: uint32 little endian]
    //                                                                       - prices of the opened essence shop
    //   C1 05 FB 03 [mode]                                                  - drop mode of the party (both directions)
    inline constexpr std::uint8_t HeadCode = 0xFB;

    void ReceivePacket(std::span<const std::uint8_t> packet);

    // Requests the change of the drop mode of the party; only the party master may change it.
    void SendPartyDropMode(GameLogic::Social::PartyDropMode::Mode mode);
}
