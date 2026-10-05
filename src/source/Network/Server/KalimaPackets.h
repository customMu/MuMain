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
    //   C1 05 FB 03 [mode]                                                  - drop mode of the party (both directions;
    //                                                                         without a party: the mode for new parties)
    //   C1 0F FB 0F [mode] [initiator name, 10 bytes]                       - a member proposes a new drop mode (vote)
    //   C1 05 FB 10 [0 = no, 1 = yes]                                       - the answer to the vote (client to server)
    //   C1 05 FB 11 [mode]                                                  - drop mode of the party of an invitation
    //   C2 [size] FB 04 [resets: u16] [tier level] [entries left] [entries per day] [seconds until reset: u32]
    //                   [can re-enter] [tier count] [0] tier count x [level, minimum resets: u16, maximum resets: u16]
    //                                                                       - entry dialog of the Kalima instance (Lugard)
    //   C1 04 FB 05                                                         - enter the Kalima instance (client to server)
    //   C2 [size] FB 06 [count: u16] count x [x, y, level: u16, monsters, name length, name: UTF-8]
    //                                                                       - monster spots of the map for the minimap
    //   C1 [size] FB 07 [count] count x [x, y]                              - spots of the Kalima instance with living monsters
    //   C1 07 FB 08 [killed packs] [packs] [boss: 0 not yet, 1 alive, 2 defeated] - progress of the Kalima instance
    //   C1 07 FB 09 [x] [y] [radius in half fields]                         - closed arena of the chamber of Kundun
    //   C2 [size] FB 0A [resets: u16] [level] [entries left] [entries per week] [seconds until reset: u32] [can re-enter]
    //                   [has lost map] [tier count] [0] tier count x [level, minimum resets: u16, maximum resets: u16]
    //                                                                       - entry dialog of the chamber of Kundun (keeper)
    //   C1 04 FB 0B                                                         - enter the chamber of Kundun (client to server)
    inline constexpr std::uint8_t HeadCode = 0xFB;

    void ReceivePacket(std::span<const std::uint8_t> packet);

    // Without a party: chooses the drop mode of new parties; in a party: proposes a change to all members.
    void SendPartyDropMode(GameLogic::Social::PartyDropMode::Mode mode);

    // The answer to a proposed change of the drop mode of the party.
    void SendPartyDropModeVoteAnswer(bool accept);

    // Enters the Kalima instance after the entry dialog was confirmed.
    void SendKalimaEnterRequest();

    // Enters the chamber of Kundun after the entry dialog of the keeper.
    void SendChamberEnterRequest();

    // The kill quests: takes the reward which waits for space in the inventory (C1 04 FB 0E).
    void SendKillQuestRewardRequest();
}
