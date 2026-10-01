#pragma once

#include <cstdint>

namespace GameLogic::Travel
{
    // Travel requirements of this server, shown and pre-checked by the client.
    // The server is the authority: it checks the map requirements (attribute "Resets") on every
    // warp and gate, and the level requirements of its warp list and enter gates.
    // Keep these tables in sync with the admin panel:
    //   - Maps -> Map requirements (Resets >= N)          -> MapResetRequirements
    //   - Warp list -> Level requirement                  -> WarpLevelRequirements
    //   - Enter gates into these maps -> Level requirement -> GateLevelRequirements
    // The client files (Movereq.bmd, Gate.bmd) still hold the original values; the entries
    // below override them.

    struct MapResetRequirement
    {
        int map; // map number (MapResetRequirements) or warp index (WarpResetRequirements)
        std::uint32_t resets;
    };

    struct LevelRequirement
    {
        int key; // warp index for warps, target map number for gates
        int level;
    };

    // Ladder of the maps by resets (session 8): a new zone every reset up to 10, then every 5.
    // A map needs the resets of its earliest zone; later zones of a map are reached by walking
    // (the monsters are the limit) or by a warp with its own requirement (WarpResetRequirements).
    inline constexpr MapResetRequirement MapResetRequirements[] =
    {
        { 7, 1 },   // Atlans
        { 4, 2 },   // Lost Tower
        { 25, 2 },  // Kalima 2
        { 26, 5 },  // Kalima 3
        { 8, 6 },   // Tarkan
        { 33, 7 },  // Aida
        { 10, 8 },  // Icarus
        { 31, 8 },  // Land of Trials
        { 27, 8 },  // Kalima 4
        { 37, 9 },  // Kanturu Ruins
        { 80, 10 }, // Karutan 1
        { 34, 10 }, // Crywolf Fortress (the entrance of Barracks of Balgass, 3rd class quest)
        { 41, 10 }, // Barracks of Balgass
        { 42, 10 }, // Balgass Refuge
        { 28, 10 }, // Kalima 5
        { 81, 20 }, // Karutan 2
        { 29, 20 }, // Kalima 6
        { 57, 30 }, // Raklion
        { 58, 30 }, // Raklion boss room
        { 36, 35 }, // Kalima 7
        { 38, 50 }, // Kanturu Relics - postponed (to be reworked as a high-end map)
        { 56, 50 }, // Swamp of Calmness - postponed (to be reworked as a high-end map)
        { 63, 50 }, // Vulcanus - postponed (to be reworked as a high-end map)
    };

    // Resets per warp list entry (server: plugin "Warp requirements by resets"); the requirement of the
    // target map applies too, the higher one counts.
    inline constexpr MapResetRequirement WarpResetRequirements[] =
    {
        { 9, 1 },   // Dungeon2
        { 10, 2 },  // Dungeon3
        { 11, 1 },  // Atlans
        { 12, 3 },  // Atlans2
        { 13, 4 },  // Atlans3
        { 14, 2 },  // LostTower
        { 15, 2 },  // LostTower2
        { 16, 3 },  // LostTower3
        { 17, 3 },  // LostTower4
        { 18, 4 },  // LostTower5
        { 19, 4 },  // LostTower6
        { 20, 5 },  // LostTower7
        { 21, 6 },  // Tarkan
        { 22, 7 },  // Tarkan2
        { 23, 8 },  // Icarus
        { 25, 7 },  // Aida1
        { 27, 15 }, // Aida2
        { 28, 9 },  // KanturuRuins1
        { 29, 10 }, // KanturuRuins2
        { 45, 25 }, // KanturuRuins3
        { 46, 10 }, // Karutan1
        { 47, 20 }, // Karutan2
        { 34, 30 }, // Raklion
        { 48, 30 }, // LaCleon
        { 30, 50 }, // KanturuRelics
        { 33, 50 }, // PeaceSwamp
        { 42, 50 }, // Vulcanus
    };

    // Close to the original levels, slightly lower: after a reset the character climbs through
    // the lower maps again before it reaches its own ones.
    inline constexpr LevelRequirement WarpLevelRequirements[] =
    {
        { 11, 60 },  // Atlans
        { 12, 70 },  // Atlans2
        { 13, 80 },  // Atlans3
        { 21, 120 }, // Tarkan
        { 22, 120 }, // Tarkan2
        { 23, 150 }, // Icarus
        { 25, 130 }, // Aida1
        { 27, 140 }, // Aida2
        { 28, 140 }, // KanturuRuins1
        { 29, 140 }, // KanturuRuins2
        { 45, 150 }, // KanturuRuins3
        { 30, 230 }, // KanturuRelics
        { 33, 400 }, // PeaceSwamp
        { 42, 400 }, // Vulcanus (index 42 in the client data; the server warp list must use the same index)
        { 46, 150 }, // Karutan1
        { 47, 160 }, // Karutan2
        { 34, 250 }, // Raklion
        { 48, 250 }, // LaCleon
    };

    // Level requirement of the enter gates into the map; replaces the value of the client file.
    inline constexpr LevelRequirement GateLevelRequirements[] =
    {
        { 7, 60 },   // Atlans
        { 8, 120 },  // Tarkan
        { 10, 150 }, // Icarus
        { 33, 130 }, // Aida
        { 37, 140 }, // Kanturu Ruins
        { 38, 230 }, // Kanturu Relics
        { 56, 400 }, // Swamp of Calmness
        { 63, 400 }, // Vulcanus
        { 80, 150 }, // Karutan 1
        { 81, 160 }, // Karutan 2
        { 57, 250 }, // Raklion
    };

    inline std::uint32_t GetRequiredResetsForMap(const int map)
    {
        for (const auto& requirement : MapResetRequirements)
        {
            if (requirement.map == map)
            {
                return requirement.resets;
            }
        }

        return 0;
    }

    // Required resets of a warp list entry: the higher of the warp and its target map.
    inline std::uint32_t GetRequiredResetsForWarp(const int warpIndex, const int targetMap)
    {
        std::uint32_t resets = GetRequiredResetsForMap(targetMap);
        for (const auto& requirement : WarpResetRequirements)
        {
            if (requirement.map == warpIndex && requirement.resets > resets)
            {
                resets = requirement.resets;
            }
        }

        return resets;
    }

    inline int GetWarpLevelRequirement(const int warpIndex, const int levelFromClientData)
    {
        for (const auto& requirement : WarpLevelRequirements)
        {
            if (requirement.key == warpIndex)
            {
                return requirement.level;
            }
        }

        return levelFromClientData;
    }

    inline int GetGateLevelRequirement(const int targetMap, const int levelFromClientData)
    {
        for (const auto& requirement : GateLevelRequirements)
        {
            if (requirement.key == targetMap)
            {
                return requirement.level;
            }
        }

        return levelFromClientData;
    }

    inline bool HasRequiredResets(const std::uint32_t resets, const int map)
    {
        return resets >= GetRequiredResetsForMap(map);
    }
}
