#pragma once

namespace GameLogic::Items
{
    // Jewels and stones which the server stacks in the inventory (up to 255 pieces per stack).
    // The number of pieces of a stack is transferred as the durability of the item.
    // Keep in sync with the item definitions of the server (ItemDefinition.Durability = 255).
    inline constexpr int MaximumJewelStackSize = 255;

    inline bool IsStackableJewel(const int itemType)
    {
        switch (itemType)
        {
        case ITEM_JEWEL_OF_BLESS:
        case ITEM_JEWEL_OF_SOUL:
        case ITEM_JEWEL_OF_LIFE:
        case ITEM_JEWEL_OF_CREATION:
        case ITEM_JEWEL_OF_CHAOS:
        case ITEM_JEWEL_OF_GUARDIAN:
        case ITEM_GEMSTONE:
        case ITEM_JEWEL_OF_HARMONY:
        case ITEM_LOWER_REFINE_STONE:
        case ITEM_HIGHER_REFINE_STONE:
        case ITEM_ILLUSION_SHARD:
            return true;
        default:
            return itemType >= ITEM_ECHO_FIRST && itemType <= ITEM_ECHO_LAST;
        }
    }
}
