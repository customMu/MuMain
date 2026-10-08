#pragma once

namespace GameLogic::Items::JewelryBox
{
    // The Jewelry Box of this server (OpenMU data update "Jewelry Box"): dropped on the ground, +1 gives one
    // excellent ring, +2 one excellent pendant. It is not in Item_<lang>.bmd: RegisterItem adds it to the item
    // table, based on the Box of Luck. Model and textures: Data/Item/JewelryBox.bmd, JewelryBox*.OZJ
    // (tools/bmd/examples/jewelry_box.py).
    inline constexpr int Number = 200;
    inline constexpr int Item = ITEM_POTION + Number;
    inline constexpr int Model = MODEL_POTION + Number;
    inline constexpr int RingLevel = 1;
    inline constexpr int PendantLevel = 2;

    // After the item table is loaded; keeps an entry the table already has.
    void RegisterItem();

    void OpenModel();
    void OpenTexture();

    // What the box gives, for the tooltip; nullptr for other levels.
    const wchar_t* GetContentText(int level);
}
