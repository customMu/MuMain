#pragma once

#include <cstdint>
#include <vector>

#include "GameLogic/Events/KalimaEntry.h"

namespace GameLogic::Events::KundunChamberEntry
{
    // The chamber of Kundun of this server (plugin "Chamber of Kundun"): talking to the keeper shows the levels by resets,
    // the Lost Map which is the entry fee and the entries left this week; the server sends these values with the custom
    // packet FB 0A, the client enters with FB 0B.
    struct Info
    {
        std::uint16_t resets = 0;
        std::uint8_t tierLevel = 0; // 0 = no chamber for these resets
        std::uint8_t entriesLeft = 0;
        std::uint8_t entriesPerWeek = 0;
        std::uint32_t secondsUntilReset = 0;
        bool canReenter = false;
        bool hasLostMap = false;
        std::vector<KalimaEntry::Tier> tiers;
    };

    void SetInfo(Info info);
    const Info& GetInfo();

    // Whether the character can enter now: a running chamber of its party, or a level, an entry left and the Lost Map.
    bool CanEnter();
}
