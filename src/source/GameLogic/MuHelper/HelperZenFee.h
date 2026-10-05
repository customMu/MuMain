#pragma once

#include <cstdint>

namespace GameLogic::MuHelper
{
    // The share of the picked up zen which the server keeps while the MU Helper runs (plugin "MU Helper zen fee",
    // Fee (%) = 20). Keep it the same as the plugin; only used for the helper statistics.
    inline constexpr std::int64_t HelperZenFeePercent = 20;

    // The fee for zen which arrived after the fee: received is (100 - fee) % of the picked up zen.
    inline std::int64_t HelperZenFeeOf(std::int64_t received)
    {
        return received <= 0 ? 0 : received * HelperZenFeePercent / (100 - HelperZenFeePercent);
    }
}
