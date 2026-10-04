#pragma once

#include "GameLogic/Events/EventTiers.h"
#include "GameLogic/Items/ItemResetRequirements.h"
#include "GameLogic/Items/PotionCooldown.h"

namespace GameLogic::Items
{
    // The resets from which a bought item can be used: armor and weapons by their grade, potions by their reset step,
    // a Lost Map +N by the reset step of the chamber of Kundun / Kalima N. 0 = no requirement.
    // The NPC shop asks for a confirmation before buying an item which the hero can't use yet.
    inline int GetRequiredResetsToUse(const int itemType, const int itemLevel)
    {
        if (itemType == ITEM_LOST_MAP)
        {
            const auto& tiers = GameLogic::Events::EventTiers::KalimaMinimumResets;
            return itemLevel >= 1 && itemLevel <= static_cast<int>(tiers.size()) ? tiers[itemLevel - 1] : 0;
        }

        if (const PotionInfo* potion = GetPotionInfo(itemType))
        {
            return potion->RequiredResets;
        }

        return GetItemRequiredResets(itemType);
    }
}
