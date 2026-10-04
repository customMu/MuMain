#include "stdafx.h"
#include "GameLogic/Events/KalimaSpots.h"

#include <utility>

#include "World/MapInfra/MapManager.h"

namespace GameLogic::Events::KalimaSpots
{
    namespace
    {
        std::vector<Spot> s_spots;
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
}
