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

    // The progress of the instance (custom packet FB 08): killed packs and the state of the boss.
    enum class BossState : std::uint8_t
    {
        NotYet = 0,
        Alive = 1,
        Defeated = 2,
    };

    struct Progress
    {
        std::uint8_t clearedPacks = 0;
        std::uint8_t packCount = 0;
        BossState bossState = BossState::NotYet;
    };

    // Replaces the progress; a pack count of 0 hides it.
    void SetProgress(Progress progress);

    // The progress while the hero is on a Kalima map and the instance sent one; nullptr otherwise.
    const Progress* GetProgress();

    // The closed arena of the chamber of Kundun (custom packet FB 09): the minimap of the whole Kalima map
    // isn't shown there. Cleared when a map is loaded.
    void SetArenaClosed(bool closed);
    bool IsArenaClosed();
}
