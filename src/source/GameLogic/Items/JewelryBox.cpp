#include "stdafx.h"
#include "GameLogic/Items/JewelryBox.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

namespace GameLogic::Items::JewelryBox
{
    void Register()
    {
        ITEM_ATTRIBUTE& box = ItemAttribute[ITEM_JEWELRY_BOX];
        box = ItemAttribute[ITEM_BOX_OF_LUCK];
        wcsncpy_s(box.Name, I18N::Game::JewelryBox, _TRUNCATE);
    }
}
