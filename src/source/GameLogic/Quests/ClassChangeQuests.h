#pragma once

// The class change quests (Sevina the Priestess, Marlon, Priest Devin) as they are on the server: what is needed,
// where it drops, the NPC. Shown in the quest window (key T, tab of the class change). Keep in sync with the server:
// Server/quest-item-drops.sql (drop levels and chances), the plugin "Quest reset requirements" (resets) and the
// quest definitions (levels).

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"

namespace GameLogic::Quests::ClassChange
{
    inline constexpr int AnyClass = -1;

    struct QuestInfo
    {
        int Quest;              // index of the legacy quest (QUEST_CHANGE_UP_1 ...)
        int MinimumLevel;
        int MinimumResets;
        const wchar_t* Npc;
        const wchar_t* NpcPlace;
    };

    // An item which drops from monsters (MonsterLevel > 0) or from bosses (Bosses).
    struct ItemGoal
    {
        int Quest;
        int BaseClass;          // CLASS_WIZARD ...; AnyClass for all
        int ItemType;
        int ItemLevel;
        int MonsterLevel;       // monsters of this level and above; 0 = bosses
        const wchar_t* Place;   // maps of the monsters / the bosses
        const wchar_t* Chance;  // per kill
    };

    struct KillGoal
    {
        int Quest;
        int MonsterNumber;
        const wchar_t* Monster;
        int Count;
        const wchar_t* Place;
    };

    inline constexpr QuestInfo Quests[] =
    {
        { QUEST_CHANGE_UP_1, 150, 1, L"Sevina the Priestess", L"Devias" },
        { QUEST_CHANGE_UP_2, 150, 1, L"Sevina the Priestess", L"Devias" },
        { QUEST_CHANGE_UP_3, 220, 3, L"Marlon", L"Lorencia, Noria, Devias, Atlans" },
        { QUEST_COMBO, 220, 3, L"Marlon", L"Lorencia, Noria, Devias, Atlans" },
        { QUEST_3RD_CHANGE_UP_1, 380, 0, L"Priest Devin", L"Devias, Crywolf" },
        { QUEST_3RD_CHANGE_UP_2, 400, 0, L"Priest Devin", L"Devias, Crywolf" },
        { QUEST_3RD_CHANGE_UP_3, 400, 0, L"Priest Devin", L"Devias, Crywolf" },
    };

    inline constexpr ItemGoal ItemGoals[] =
    {
        { QUEST_CHANGE_UP_1, AnyClass, ITEM_POTION + 23, 0, 50, L"Atlans 1", L"0.1%" },               // Scroll of Emperor
        { QUEST_CHANGE_UP_2, CLASS_WIZARD, ITEM_POTION + 26, 0, 60, L"Atlans 1", L"0.05%" },          // Soul Shard of Wizard
        { QUEST_CHANGE_UP_2, CLASS_KNIGHT, ITEM_POTION + 24, 0, 60, L"Atlans 1", L"0.05%" },          // Broken Sword
        { QUEST_CHANGE_UP_2, CLASS_ELF, ITEM_POTION + 25, 0, 60, L"Atlans 1", L"0.05%" },             // Tear of Elf
        { QUEST_CHANGE_UP_2, CLASS_SUMMONER, ITEM_POTION + 68, 0, 60, L"Atlans 1", L"0.05%" },        // Eye of Abyssal
        { QUEST_CHANGE_UP_3, AnyClass, ITEM_POTION + 23, 1, 90, L"Atlans 2, Lost Tower", L"0.1%" },   // Ring of Honor
        { QUEST_COMBO, AnyClass, ITEM_POTION + 24, 1, 103, L"Atlans 2, Lost Tower", L"0.05%" },       // Dark Stone
        { QUEST_3RD_CHANGE_UP_1, AnyClass, ITEM_POTION + 65, 0, 0, L"Death Beam Knight, Tarkan", L"100%" },  // Flame of Death Beam Knight
        { QUEST_3RD_CHANGE_UP_1, AnyClass, ITEM_POTION + 66, 0, 0, L"Hell Maine, Aida", L"100%" },           // Horn of Hell Maine
        { QUEST_3RD_CHANGE_UP_1, AnyClass, ITEM_POTION + 67, 0, 0, L"Dark Phoenix, Icarus", L"100%" },       // Feather of Dark Phoenix
    };

    inline constexpr KillGoal KillGoals[] =
    {
        { QUEST_3RD_CHANGE_UP_2, 409, L"Balram", 20, L"Barracks of Balgass" },
        { QUEST_3RD_CHANGE_UP_2, 410, L"Death Spirit", 20, L"Barracks of Balgass" },
        { QUEST_3RD_CHANGE_UP_2, 411, L"Soram", 20, L"Barracks of Balgass" },
        { QUEST_3RD_CHANGE_UP_3, 412, L"Dark Elf", 1, L"Balgass Refuge" },
    };

    inline const QuestInfo* FindQuest(const int quest)
    {
        for (const auto& info : Quests)
        {
            if (info.Quest == quest)
            {
                return &info;
            }
        }

        return nullptr;
    }
}
