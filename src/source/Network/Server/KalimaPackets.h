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
    //   C2 [size] FB 04 [resets: u16] [tier level] [entries left] [entries per day] [seconds until reset: u32]
    //                   [can re-enter] [tier count] [0] tier count x [level, minimum resets: u16, maximum resets: u16]
    //                                                                       - entry dialog of the Kalima instance (Lugard)
    //   C1 04 FB 05                                                         - enter the Kalima instance (client to server)
    //   C2 [size] FB 06 [count: u16] count x [x, y, level: u16, monsters, name length, name: UTF-8]
    //                                                                       - monster spots of the map for the minimap
    inline constexpr std::uint8_t HeadCode = 0xFB;

    void ReceivePacket(std::span<const std::uint8_t> packet);

    // Requests the change of the drop mode of the party; only the party master may change it.
    void SendPartyDropMode(GameLogic::Social::PartyDropMode::Mode mode);

    // Enters the Kalima instance after the entry dialog was confirmed.
    void SendKalimaEnterRequest();
}
