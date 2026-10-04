#pragma once

#include <array>

namespace GameLogic::Events::EventTiers
{
    // Blood Castle and Devil Square by resets (server plugin "Events by resets"): the minimum resets of the levels 1..N.
    // A level reaches up to the minimum of the next level minus one. Keep in sync with the server configuration.
    // Blood Castle and Devil Square have the same levels 1-7; Blood Castle 8 is for 50 resets.
    inline constexpr std::array<int, 8> BloodCastleMinimumResets = { 0, 5, 10, 15, 22, 30, 38, 50 };
    inline constexpr std::array<int, 7> DevilSquareMinimumResets = { 0, 5, 10, 15, 22, 30, 38 };

    // Kalima 1-7 and the chamber of Kundun 1-7 (plugin "Kalima instance", tiers): the minimum resets of the levels.
    inline constexpr std::array<int, 7> KalimaMinimumResets = { 5, 10, 15, 22, 30, 38, 45 };

    // The index (0-based) of the level for the reset count.
    template <std::size_t N>
    int GetLevelIndex(const std::array<int, N>& minimumResets, const int resets)
    {
        int index = 0;
        for (int i = 0; i < static_cast<int>(N); ++i)
        {
            if (resets >= minimumResets[i])
            {
                index = i;
            }
        }

        return index;
    }
}
