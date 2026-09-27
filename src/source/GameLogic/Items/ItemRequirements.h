#pragma once

#include <cstdint>
#include <limits>

namespace GameLogic::Items
{
    // Item requirement rules of the server plugin "Item requirements by base stats" (ItemRequirementsByBaseStatsPlugIn):
    // - stat requirements are checked against the base stats (the distributed points, CharacterAttribute->Strength etc.),
    //   not against the total stats including the bonuses of items (CharacterAttribute->AddStrength etc.);
    // - the level requirement of wearable items is ignored after the first reset.
    // An equipped item which doesn't meet them stays equipped, but gives no bonuses (see IsRequireEquipItem).

    // The level which is compared with the level requirement of a wearable item.
    inline std::uint16_t GetLevelForEquipmentRequirement(const std::uint16_t level, const std::uint32_t resets)
    {
        return resets > 0 ? (std::numeric_limits<std::uint16_t>::max)() : level;
    }

    inline bool IsEquipmentSlot(const int itemSlot)
    {
        return itemSlot >= 0 && itemSlot < MAX_EQUIPMENT_INDEX;
    }
}
