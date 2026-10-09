#pragma once

#include <algorithm>

#include "GameLogic/Items/ArmorRanks.h"
#include "GameLogic/Items/WeaponRanks.h"

namespace GameLogic::Items
{
    // The price of enchanting by rank (wiki §7, 09.10.2026). Jewel of Bless / Soul: ranks 3..8 take 1, 2, 3, 4, 5, 6
    // jewels of the stack per level; ranks 1 and 2: 1 Bless gives +3 / +2 levels (not above +6), 1 Soul one level. Chaos
    // Machine +10..+15: the Bless and Soul of the mix x1 (ranks 1-3), x2 (4-5), x3 (6-8). The first sets of a rank
    // (Dragon, Guardian, Dark Phoenix, Valiant, Glorious) have the price of the rank before. Items without a rank
    // (shields, wings, rings ...): 1 jewel per level, x1. The server has the same (tools/balance/enchant_price.py ->
    // GameLogic/Items/EnchantPriceRanks.cs).
    inline constexpr int EarlySets[] = { 1, 14, 17, 37, 38 }; // the number of the set in the groups 7..11

    // The rank of the price: the rank of the item, one lower for the first sets of a rank, 0 without a rank.
    inline int GetEnchantPriceRank(const int itemType)
    {
        if (const auto* armor = FindArmorRank(itemType))
        {
            const int number = itemType % MAX_ITEM_INDEX;
            const bool early = std::find(std::begin(EarlySets), std::end(EarlySets), number) != std::end(EarlySets);
            return std::max(1, armor->Rank - (early ? 1 : 0));
        }

        if (const auto* weapon = FindWeaponRank(itemType))
        {
            return weapon->Rank;
        }

        return 0;
    }

    struct EnchantStep
    {
        int Jewels; // taken from the stack
        int Levels; // the levels one use gives (Bless of ranks 1-2)
    };

    // One use of Jewel of Bless (soul = false) or Jewel of Soul (soul = true) on the item.
    inline EnchantStep GetEnchantStep(const int itemType, const bool soul)
    {
        const int rank = GetEnchantPriceRank(itemType);
        if (rank <= 2)
        {
            return { 1, soul || rank == 0 ? 1 : 4 - rank };
        }

        return { rank - 2, 1 };
    }

    // The multiplier of the Bless and Soul of the Chaos Machine mixes +10..+15.
    inline int GetChaosMachineJewelMultiplier(const int itemType)
    {
        const int rank = GetEnchantPriceRank(itemType);
        return rank >= 6 ? 3 : rank >= 4 ? 2 : 1;
    }
}
