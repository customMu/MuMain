#include "stdafx.h"
#include "GameLogic/Quests/KillQuest.h"

#include <utility>

#include "Network/Server/KalimaPackets.h"

namespace GameLogic::Quests::KillQuest
{
    namespace
    {
        State s_state;
    }

    void SetState(State state)
    {
        state.known = true;
        s_state = std::move(state);
    }

    const State& GetState()
    {
        return s_state;
    }

    bool IsRewardWaiting()
    {
        return s_state.known && s_state.rewardWaiting;
    }

    bool IsAllDone()
    {
        return s_state.known && s_state.number > s_state.count;
    }

    void RequestReward()
    {
        Network::Server::KalimaPackets::SendKillQuestRewardRequest();
    }
}
