#pragma once

#include <cstdint>
#include <vector>

namespace GameLogic::Events::KalimaEntry
{
    // The Kalima instance of this server (plugin "Kalima instance"): talking to Lugard shows which Kalima
    // the character can enter by its resets and how many entries are left today; the server sends these
    // values with the custom packet FB 04, the client enters with FB 05.
    inline constexpr std::uint16_t OpenEnd = 0xFFFF;

    struct Tier
    {
        std::uint8_t level;
        std::uint16_t minimumResets;
        std::uint16_t maximumResets; // OpenEnd for the last tier
    };

    struct Info
    {
        std::uint16_t resets = 0;
        std::uint8_t tierLevel = 0; // 0 = no Kalima for these resets
        std::uint8_t entriesLeft = 0;
        std::uint8_t entriesPerDay = 0;
        std::uint32_t secondsUntilReset = 0;
        bool canReenter = false;
        std::vector<Tier> tiers;
    };

    void SetInfo(Info info);
    const Info& GetInfo();

    // Whether the character can enter now: a running instance of its party, or a tier and an entry left.
    bool CanEnter();
}
