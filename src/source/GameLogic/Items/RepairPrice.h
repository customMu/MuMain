#pragma once

#include <cstdint>

namespace GameLogic::Items
{
    // Flat repair price per missing durability point. The server applies it with the plugin
    // "Repair price" (RepairPricePlugIn); its prices are configurable in the admin panel, so keep
    // these defaults in sync with the server configuration.
    inline constexpr int RepairPricePerDurability = 50;
    inline constexpr int RepairWingsPricePerDurability = 100;
    inline constexpr float RepairSelfRepairMultiplier = 2.5f;
    inline constexpr float RepairBrokenItemMultiplier = 1.4f;

    // Dark Horse, Dark Raven and ammunition keep the original, value based price.
    inline bool HasFlatRepairPrice(const short type)
    {
        return type != ITEM_DARK_HORSE_ITEM && type != ITEM_DARK_RAVEN_ITEM
            && type != ITEM_BOLT && type != ITEM_ARROWS;
    }

    inline bool IsWingsRepairPrice(const short type)
    {
        return (type >= ITEM_WING && type < ITEM_HELPER) || type == ITEM_CAPE_OF_LORD;
    }

    inline std::int64_t CalcFlatRepairPrice(const short type, const int durability, const int maxDurability, const bool selfRepair)
    {
        const int missing = maxDurability - durability;
        if (missing <= 0)
        {
            return 0;
        }

        const int pricePerDurability = IsWingsRepairPrice(type) ? RepairWingsPricePerDurability : RepairPricePerDurability;
        double price = static_cast<double>(pricePerDurability) * missing;
        if (durability <= 0)
        {
            price *= RepairBrokenItemMultiplier;
        }

        if (selfRepair)
        {
            price *= RepairSelfRepairMultiplier;
        }

        auto result = static_cast<std::int64_t>(price);
        if (result >= 1000)
        {
            result = result / 100 * 100;
        }
        else if (result >= 100)
        {
            result = result / 10 * 10;
        }

        return result;
    }
}
