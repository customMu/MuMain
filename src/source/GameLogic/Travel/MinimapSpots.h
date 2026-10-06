#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace GameLogic::Travel::MinimapSpots
{
    // The monster spots of the current map (plugin "Minimap monster spots" of this server): the server sends
    // them with the custom packet FB 06 when the character entered a map; the minimap draws them as dots.
    struct Spot
    {
        std::uint8_t x;
        std::uint8_t y;
        std::uint16_t level;
        std::uint8_t count;
        std::wstring name;
        bool boss = false;
    };

    void SetSpots(std::vector<Spot> spots);
    void Clear();
    const std::vector<Spot>& GetSpots();
}
