#include "stdafx.h"
#include "GameLogic/Travel/MinimapSpots.h"

#include <utility>

namespace GameLogic::Travel::MinimapSpots
{
    namespace
    {
        std::vector<Spot> s_spots;
    }

    void SetSpots(std::vector<Spot> spots)
    {
        s_spots = std::move(spots);
    }

    void Clear()
    {
        s_spots.clear();
    }

    const std::vector<Spot>& GetSpots()
    {
        return s_spots;
    }
}
