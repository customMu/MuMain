#pragma once

#include <cstdint>

namespace GameLogic::Social::PartyDropMode
{
    // The drop mode of the party (plugin of this server): who of the party may pick up the items
    // which drop for kills of the party. The party master changes it, the server sends it to all members.
    enum class Mode : std::uint8_t
    {
        Free = 0,
        Random = 1,
        InTurn = 2,
    };

    void SetMode(Mode mode);
    Mode GetMode();

    // True, after the server sent the mode, i.e. the server supports the drop modes.
    bool IsKnown();

    Mode GetNextMode(Mode mode);
    // The I18N slot of the label of the mode, for CNewUIButton::ChangeText.
    const wchar_t* const* GetModeText(Mode mode);
}
