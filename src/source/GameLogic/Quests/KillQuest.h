#pragma once

#include <cstdint>
#include <string>

// The kill quests of the server (plugin "Kill quests"): the current quest of the character, from the custom packet
// FB 0D, shown in the quest window (key T). A reward which waits for space in the inventory is taken with FB 0E.
namespace GameLogic::Quests::KillQuest
{
    struct State
    {
        bool known = false;
        int number = 0;          // from 1; count + 1 when all quests are done
        int count = 0;
        int kills = 0;
        int killsNeeded = 0;
        int rewardPoints = 0;
        bool rewardWaiting = false;
        int questPoints = 0;
        std::wstring monster;
        std::wstring reward;     // the items of the reward, one per line
    };

    void SetState(State state);
    const State& GetState();
    bool IsRewardWaiting();
    bool IsAllDone();

    // Asks the server for the waiting reward (FB 0E).
    void RequestReward();
}
