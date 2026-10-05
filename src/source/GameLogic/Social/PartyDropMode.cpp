#include "stdafx.h"
#include "GameLogic/Social/PartyDropMode.h"

#include "I18N/All.h"
#include "Data/GameConfig/GameConfig.h"

namespace GameLogic::Social::PartyDropMode
{
    namespace
    {
        Mode g_Mode = Mode::Free;
        bool g_IsKnown = false;
        Mode g_InviteMode = Mode::Free;
        Mode g_VoteMode = Mode::Free;
        std::wstring g_VoteInitiator;

        Mode Validate(const Mode mode)
        {
            return mode <= Mode::InTurn ? mode : Mode::Free;
        }
    }

    Mode GetPreferredMode()
    {
        return Validate(static_cast<Mode>(GameConfig::GetInstance().GetPartyDropMode()));
    }

    void SetPreferredMode(const Mode mode)
    {
        GameConfig::GetInstance().SetPartyDropMode(static_cast<int>(Validate(mode)));
        GameConfig::GetInstance().Save();
    }

    void SetInviteMode(const Mode mode)
    {
        g_InviteMode = Validate(mode);
    }

    Mode GetInviteMode()
    {
        return g_InviteMode;
    }

    void SetVote(const Mode mode, std::wstring initiator)
    {
        g_VoteMode = Validate(mode);
        g_VoteInitiator = std::move(initiator);
    }

    Mode GetVoteMode()
    {
        return g_VoteMode;
    }

    const std::wstring& GetVoteInitiator()
    {
        return g_VoteInitiator;
    }

    void SetMode(const Mode mode)
    {
        g_Mode = mode <= Mode::InTurn ? mode : Mode::Free;
        g_IsKnown = true;
    }

    Mode GetMode()
    {
        return g_Mode;
    }

    bool IsKnown()
    {
        return g_IsKnown;
    }

    Mode GetNextMode(const Mode mode)
    {
        switch (mode)
        {
        case Mode::Free:
            return Mode::Random;
        case Mode::Random:
            return Mode::InTurn;
        default:
            return Mode::Free;
        }
    }

    const wchar_t* const* GetModeText(const Mode mode)
    {
        switch (mode)
        {
        case Mode::Random:
            return &I18N::Game::DropRandom;
        case Mode::InTurn:
            return &I18N::Game::DropInTurn;
        default:
            return &I18N::Game::DropFree;
        }
    }
}
