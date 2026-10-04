#pragma once

#include <cstdint>
#include <vector>

namespace GameLogic::Events::KalimaSpots
{
    // The spots of the Kalima instance (plugin "Kalima instance") which still have living monsters: the
    // current pack, or the Illusion of Kundun after the last pack. The server sends them with the custom
    // packet FB 06 whenever they change; the minimap shows them.
    struct Spot
    {
        std::uint8_t x;
        std::uint8_t y;
    };

    // Replaces the spots. Each entry into the instance sends them again, so older ones are never shown.
    void SetSpots(std::vector<Spot> spots);

    // The spots while the hero is on a Kalima map; empty on any other map.
    const std::vector<Spot>& GetSpots();
}
