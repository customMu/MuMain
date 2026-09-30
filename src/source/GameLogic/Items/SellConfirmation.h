#pragma once

#include "Engine/Object/ZzzInventory.h"

namespace GameLogic::Items
{
    // Every weapon, shield and armor piece is rare on this server (items drop at +0 only),
    // so selling one of them to an NPC always asks for a confirmation, not only the
    // high value items (jewels, wings, excellent, ancient, +7 and higher).
    inline bool IsWeaponOrArmor(const ITEM* pItem)
    {
        return pItem != nullptr
            && pItem->Type >= ITEM_SWORD
            && pItem->Type <= ITEM_BOOTS + MAX_ITEM_INDEX - 1;
    }

    inline bool NeedsSellConfirmation(ITEM* pItem)
    {
        return IsHighValueItem(pItem) || IsWeaponOrArmor(pItem);
    }
}
