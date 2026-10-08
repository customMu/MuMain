#pragma once

namespace GameLogic::Items::JewelryBox
{
    // The Jewelry Box (14/170) is the reward of the last kill quests (server plugin "Kill quests"):
    // dropped on the ground, +1 gives one random excellent ring and +2 one random excellent pendant
    // (server: JewelryBox.cs, update "Jewelry box"). Model: Data\Item\JewelryBox01.bmd from
    // tools/bmd/examples/jewelry_box.py.
    inline constexpr int RingLevel = 1;
    inline constexpr int PendantLevel = 2;

    // The item is not in Item_<lang>.bmd: takes the attributes of the Box of Luck and its own name.
    // Called right after the item file is loaded.
    void Register();
}
