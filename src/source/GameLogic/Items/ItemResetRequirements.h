#pragma once

#include "GameLogic/Items/ArmorRanks.h"
#include "GameLogic/Items/WeaponRanks.h"

namespace GameLogic::Items
{
    // Armor and weapons are worn from the reset of their rank (wiki §5); rank 4 from 5 resets (as the chaos weapons). The server has the same values as
    // item requirements "Resets >= N" (Server/item-reset-requirements.sql, tools/balance/item_reset_sql.py).
    inline constexpr int RankStartResets[8] = { 0, 1, 3, 5, 10, 15, 20, 26 };

    // Sets which start later (or earlier) than their rank; the number is the set index, the same in the groups 7..11.
    struct SetResetStart
    {
        int number;
        int resets;
    };

    inline constexpr SetResetStart SetResetStarts[] =
    {
        { 34, 12 }, // Ashcrow (rank 5)
        { 36, 12 }, // Iris (rank 5)
        { 20, 22 }, // Thunder Hawk (rank 7)
        { 21, 23 }, // Great Dragon (rank 7)
        { 28, 23 }, // Dark Master (rank 7)
        { 23, 24 }, // Hurricane (rank 7)
    };

    // Weapons which are worn from another reset than their rank starts (item type, resets).
    struct WeaponResetStart
    {
        int type;
        int resets;
    };

    inline constexpr WeaponResetStart WeaponResetStarts[] =
    {
        { ITEM_MACE + 6, 5 },  // Chaos Dragon Axe (rank 3, Blood Castle / Devil Square level 2)
        { ITEM_BOW + 6, 5 },   // Chaos Nature Bow
        { ITEM_STAFF + 7, 5 }, // Chaos Lightning Staff
    };

    // Rank 1..8 of armor and weapons, 0 for other items.
    inline int GetItemRank(const int itemType)
    {
        if (const auto* armor = FindArmorRank(itemType))
        {
            return armor->Rank;
        }

        if (const auto* weapon = FindWeaponRank(itemType))
        {
            return weapon->Rank;
        }

        return 0;
    }

    // Resets needed to wear the item, 0 if it has no rank.
    inline int GetItemRequiredResets(const int itemType)
    {
        const int rank = GetItemRank(itemType);
        if (rank < 1 || rank > 8)
        {
            return 0;
        }

        if (FindArmorRank(itemType) != nullptr)
        {
            const int number = itemType % MAX_ITEM_INDEX;
            for (const auto& start : SetResetStarts)
            {
                if (start.number == number)
                {
                    return start.resets;
                }
            }
        }

        for (const auto& start : WeaponResetStarts)
        {
            if (start.type == itemType)
            {
                return start.resets;
            }
        }

        return RankStartResets[rank - 1];
    }
}
