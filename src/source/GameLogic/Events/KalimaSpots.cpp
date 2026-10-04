#include "stdafx.h"
#include "GameLogic/Events/KalimaSpots.h"

#include <utility>

#include "World/MapInfra/MapManager.h"

namespace GameLogic::Events::KalimaSpots
{
    namespace
    {
        std::vector<Spot> s_spots;
        Progress s_progress;
        bool s_arenaClosed = false;
    }

    void SetSpots(std::vector<Spot> spots)
    {
        s_spots = std::move(spots);
    }

    const std::vector<Spot>& GetSpots()
    {
        static const std::vector<Spot> none;
        return gMapManager.InHellas() ? s_spots : none;
    }

    void SetProgress(const Progress progress)
    {
        s_progress = progress;
    }

    const Progress* GetProgress()
    {
        return gMapManager.InHellas() && s_progress.packCount > 0 ? &s_progress : nullptr;
    }

    void SetArenaClosed(const bool closed)
    {
        s_arenaClosed = closed;
    }

    bool IsArenaClosed()
    {
        return s_arenaClosed && gMapManager.InHellas();
    }
}
