#include "stdafx.h"
#include "GameLogic/Items/SetGuard.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "Character/CharacterManager.h"
#include "I18N/All.h"

namespace GameLogic::Items::SetGuard
{
    namespace
    {
        constexpr int ArmorGroups[5] = { ITEM_GROUP_HELM, ITEM_GROUP_ARMOR, ITEM_GROUP_PANTS, ITEM_GROUP_GLOVES, ITEM_GROUP_BOOTS };
        constexpr int ArmorSlots[5] = { EQUIPMENT_HELM, EQUIPMENT_ARMOR, EQUIPMENT_PANTS, EQUIPMENT_GLOVES, EQUIPMENT_BOOTS };

        bool IsArmor(const int itemType)
        {
            return itemType >= ITEM_HELM && itemType < ITEM_BOOTS + MAX_ITEM_INDEX;
        }

        // A Magic Gladiator wears no helm, a Rage Fighter no gloves.
        bool IsSlotRequired(const int group)
        {
            const auto baseClass = gCharacterManager.GetBaseClass(CharacterAttribute->Class);
            if (baseClass == CLASS_DARK && group == ITEM_GROUP_HELM)
            {
                return false;
            }

            if (baseClass == CLASS_RAGEFIGHTER && group == ITEM_GROUP_GLOVES)
            {
                return false;
            }

            return true;
        }

        float GetMaximumPercent(const SetInfo& set)
        {
            return GetPercent(set, StepLevels[4]);
        }

        void AddLine(int& textNum, const int color, const wchar_t* text)
        {
            mu_swprintf(TextList[textNum], L"%ls", text);
            TextListColor[textNum] = color;
            TextBold[textNum] = false;
            ++textNum;
        }
    }

    const SetInfo* FindSet(const int itemType)
    {
        if (!IsArmor(itemType))
        {
            return nullptr;
        }

        const int number = itemType % MAX_ITEM_INDEX;
        for (const auto& set : Sets)
        {
            if (set.Number == number)
            {
                return &set;
            }
        }

        return nullptr;
    }

    int GetRequiredParts(const SetInfo& set)
    {
        int parts = 0;
        for (const int group : ArmorGroups)
        {
            const int type = group * MAX_ITEM_INDEX + set.Number;
            if (ItemAttribute[type].Name[0] != 0 && IsSlotRequired(group))
            {
                ++parts;
            }
        }

        return parts;
    }

    float GetPercent(const SetInfo& set, const int lowestLevel)
    {
        if (set.Rank < 1 || set.Rank > 8)
        {
            return 0.f;
        }

        int steps = 0;
        for (const int level : StepLevels)
        {
            if (lowestLevel >= level)
            {
                ++steps;
            }
        }

        return RankPercent[set.Rank - 1] * (1.f + steps * StepBonus);
    }

    int GetNextStepLevel(const int lowestLevel)
    {
        for (const int level : StepLevels)
        {
            if (lowestLevel < level)
            {
                return level;
            }
        }

        return -1;
    }

    State GetHeroState()
    {
        State state;
        if (CharacterMachine == nullptr)
        {
            return state;
        }

        int lowestLevel = 15;
        for (const int slot : ArmorSlots)
        {
            ITEM* item = &CharacterMachine->Equipment[slot];
            if (item->Type == -1)
            {
                continue;
            }

            const SetInfo* set = FindSet(item->Type);
            if (set == nullptr || (state.Set != nullptr && state.Set != set))
            {
                // pieces of different sets don't count together
                return State {};
            }

            state.Set = set;
            if (item->Durability > 0 && IsRequireEquipItem(item))
            {
                ++state.WornParts;
                lowestLevel = (std::min)(lowestLevel, static_cast<int>(item->Level));
            }
        }

        if (state.Set == nullptr)
        {
            return state;
        }

        state.RequiredParts = GetRequiredParts(*state.Set);
        state.IsComplete = state.RequiredParts > 0 && state.WornParts == state.RequiredParts;
        state.LowestLevel = state.IsComplete ? lowestLevel : 0;
        state.Percent = state.IsComplete ? GetPercent(*state.Set, lowestLevel) : 0.f;
        return state;
    }

    int AppendTooltip(ITEM* item, int textNum)
    {
        const SetInfo* set = item != nullptr ? FindSet(item->Type) : nullptr;
        if (set == nullptr || CharacterMachine == nullptr)
        {
            return textNum;
        }

        bool isEquipped = false;
        for (const int slot : ArmorSlots)
        {
            if (item == &CharacterMachine->Equipment[slot])
            {
                isEquipped = true;
                break;
            }
        }

        wchar_t line[100] {};
        AddLine(textNum, TEXT_COLOR_WHITE, L"\n");
        if (!isEquipped)
        {
            mu_swprintf(line, I18N::Game::SetGuardLsSetDParts1fTo1fAt15, set->Name, GetRequiredParts(*set), GetPercent(*set, 0), GetMaximumPercent(*set));
            AddLine(textNum, TEXT_COLOR_GRAY, line);
            return textNum;
        }

        const State state = GetHeroState();
        if (!state.IsComplete || state.Set != set)
        {
            const int worn = state.Set == set ? state.WornParts : 1;
            mu_swprintf(line, I18N::Game::SetGuardLsSetDDParts, set->Name, worn, GetRequiredParts(*set));
            AddLine(textNum, TEXT_COLOR_BLUE, line);
            mu_swprintf(line, I18N::Game::FullSetIncomingDamage1f, GetPercent(*set, 0));
            AddLine(textNum, TEXT_COLOR_GRAY, line);
            return textNum;
        }

        return AppendBuffTooltip(textNum);
    }

    int AppendBuffTooltip(int textNum)
    {
        const State state = GetHeroState();
        if (!state.IsComplete)
        {
            return textNum;
        }

        wchar_t line[100] {};
        mu_swprintf(line, I18N::Game::SetGuardLsSetDDPartsLowestD, state.Set->Name, state.WornParts, state.RequiredParts, state.LowestLevel);
        AddLine(textNum, TEXT_COLOR_BLUE, line);
        mu_swprintf(line, I18N::Game::IncomingDamage1f, state.Percent);
        AddLine(textNum, TEXT_COLOR_WHITE, line);
        const int nextLevel = GetNextStepLevel(state.LowestLevel);
        if (nextLevel > 0)
        {
            mu_swprintf(line, I18N::Game::NextStepAtD1f, nextLevel, GetPercent(*state.Set, nextLevel));
            AddLine(textNum, TEXT_COLOR_GRAY, line);
        }

        return textNum;
    }
}
