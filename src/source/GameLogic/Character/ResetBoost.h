#pragma once

#include <cstdint>

namespace GameLogic::Character
{
    // Passive bonus per reset, shown in the character window. The server applies it with the plugin
    // "Reset boost" (ResetBoostPlugIn); its percentages are configurable in the admin panel, so keep
    // these defaults in sync with the server configuration.
    inline constexpr float ResetBoostHealthAndManaPercentPerReset = 0.4f;
    inline constexpr float ResetBoostDamageAndDefensePercentPerReset = 0.3f;

    inline float GetResetBoostHealthAndManaPercent(const std::uint32_t resets)
    {
        return static_cast<float>(resets) * ResetBoostHealthAndManaPercentPerReset;
    }

    inline float GetResetBoostDamageAndDefensePercent(const std::uint32_t resets)
    {
        return static_cast<float>(resets) * ResetBoostDamageAndDefensePercentPerReset;
    }
}
