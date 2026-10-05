#pragma once

#include <cstdint>
#include <string>

namespace GameLogic::Social::PartyDropMode
{
    // The drop mode of the party (plugin of this server): who of the party may pick up the items
    // which drop for kills of the party. Without a party the player chooses the mode of the parties which it creates
    // (saved in the config); in a party a change needs the agreement of all members (vote).
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

    // The mode chosen for new parties (config); sent to the server when the hero enters the world.
    Mode GetPreferredMode();
    void SetPreferredMode(Mode mode);

    // The drop mode of the party to which the hero is invited (sent by the server before the invitation).
    void SetInviteMode(Mode mode);
    Mode GetInviteMode();

    // A member proposes a change of the drop mode of the party.
    void SetVote(Mode mode, std::wstring initiator);
    Mode GetVoteMode();
    const std::wstring& GetVoteInitiator();
}
