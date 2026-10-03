#include "stdafx.h"
#include "GameLogic/Social/PartyDropMode.h"

#include "I18N/All.h"

namespace GameLogic::Social::PartyDropMode
{
    namespace
    {
        Mode g_Mode = Mode::Free;
        bool g_IsKnown = false;
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
