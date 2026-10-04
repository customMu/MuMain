#pragma once

namespace GameLogic::Travel::MinimapWalk
{
    // Walking to a point which was clicked on the minimap. The path finder of the client returns at most
    // 15 steps, so the hero walks in segments: each time a segment ended, the next one is searched from
    // the current position, like the MU Helper returns to its spot. A click on the ground, the MU Helper,
    // the death of the hero or a map change ends it.

    // Returns false when the field can't be walked on.
    bool Start(int x, int y);
    void Cancel();
    bool IsActive();
    bool GetTarget(int& x, int& y);

    // Called every frame while the hero can move.
    void Process();
}
