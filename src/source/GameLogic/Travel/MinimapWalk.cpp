#include "stdafx.h"
#include "GameLogic/Travel/MinimapWalk.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "MUHelper/MuHelper.h"
#include "Render/Terrain/ZzzLodTerrain.h"

extern int TargetX, TargetY;

namespace GameLogic::Travel::MinimapWalk
{
    namespace
    {
        constexpr DWORD RetryIntervalMs = 150;
        constexpr int MaximumFailures = 6;
        constexpr float ArrivalRange = 1.5f;

        struct State
        {
            bool active = false;
            int x = 0;
            int y = 0;
            int failures = 0;
            int lastX = -1;
            int lastY = -1;
            DWORD lastTry = 0;
        };

        State s_state;

        bool IsWalkable(int x, int y)
        {
            if (x < 0 || x > 255 || y < 0 || y > 255)
            {
                return false;
            }

            const WORD attribute = TerrainWall[TERRAIN_INDEX(x, y)];
            return (attribute & TW_ACTION) == TW_ACTION || (attribute & (TW_NOMOVE | TW_NOGROUND)) == 0;
        }
    }

    bool Start(const int x, const int y)
    {
        if (Hero == nullptr || !IsWalkable(x, y))
        {
            return false;
        }

        s_state = State{ true, x, y };
        return true;
    }

    void Cancel()
    {
        s_state.active = false;
    }

    bool IsActive()
    {
        return s_state.active;
    }

    bool GetTarget(int& x, int& y)
    {
        x = s_state.x;
        y = s_state.y;
        return s_state.active;
    }

    void Process()
    {
        if (!s_state.active || Hero == nullptr)
        {
            return;
        }

        if (!Hero->Object.Live || MUHelper::g_MuHelper.IsActive())
        {
            Cancel();
            return;
        }

        if (Hero->Movement)
        {
            return;
        }

        const DWORD now = GetTickCount();
        if (now - s_state.lastTry < RetryIntervalMs)
        {
            return;
        }

        s_state.lastTry = now;
        TargetX = s_state.x;
        TargetY = s_state.y;
        if (CheckTile(Hero, &Hero->Object, ArrivalRange))
        {
            Cancel();
            return;
        }

        // No progress since the last segment (e.g. the target can't be reached): give up after a few tries.
        if (Hero->PositionX == s_state.lastX && Hero->PositionY == s_state.lastY && ++s_state.failures > MaximumFailures)
        {
            Cancel();
            return;
        }

        s_state.lastX = Hero->PositionX;
        s_state.lastY = Hero->PositionY;
        Hero->MovementType = MOVEMENT_MOVE;
        if (PathFinding2(Hero->PositionX, Hero->PositionY, s_state.x, s_state.y, &Hero->Path))
        {
            SendMove(Hero, &Hero->Object);
        }
        else if (++s_state.failures > MaximumFailures)
        {
            Cancel();
        }
    }
}
