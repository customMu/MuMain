#include "stdafx.h"
#include "GameLogic/Items/JewelryBox.h"

#include "Data/DataHandler/LoadData.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

namespace GameLogic::Items::JewelryBox
{
    void RegisterItem()
    {
        ITEM_ATTRIBUTE& box = ItemAttribute[Item];
        if (box.Name[0] != L'\0')
        {
            return;
        }

        box = ItemAttribute[ITEM_BOX_OF_LUCK];
        wcsncpy(box.Name, I18N::Game::JewelryBox, MAX_ITEM_NAME - 1);
        box.Name[MAX_ITEM_NAME - 1] = L'\0';
    }

    void OpenModel()
    {
        gLoadData.AccessModel(Model, L"Data\\Item\\", L"JewelryBox");
    }

    void OpenTexture()
    {
        gLoadData.OpenTexture(Model, L"Item\\");
    }

    const wchar_t* GetContentText(const int level)
    {
        switch (level)
        {
        case RingLevel:
            return I18N::Game::YouGetOneExcellentRing;
        case PendantLevel:
            return I18N::Game::YouGetOneExcellentPendant;
        default:
            return nullptr;
        }
    }
}
