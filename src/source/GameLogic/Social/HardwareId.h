#pragma once

#include <array>
#include <cstdint>

namespace GameLogic::Social::HardwareId
{
    // A fingerprint of this computer for the rankings of the server: kills between two characters played on the same
    // computer don't count (plugin "Ranking statistics"). It is a hash of the Windows MachineGuid and the serial number
    // of the system drive - the identifiers themselves never leave the computer. Sent at the entry into the game
    // (KalimaPackets::SendHardwareId, C1 14 FB 20).
    const std::array<std::uint8_t, 16>& Get();
}
