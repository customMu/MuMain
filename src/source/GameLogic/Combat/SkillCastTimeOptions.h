#pragma once

#include <algorithm>
#include <iterator>

#include "GameLogic/Combat/SkillCastTime.h"

namespace GameLogic::Combat::SkillCastTime
{
    // The weapon entry of an item for a class family (number of the 1st class), or nullptr: an entry of the family
    // wins over one for any class (Staff of Kundun: DW and MG have their own skills).
    inline const WeaponSkills* FindWeapon(int group, int number, int family)
    {
        const WeaponSkills* found = nullptr;
        for (const auto& entry : Weapons)
        {
            if (entry.Group == group && entry.Number == number && (entry.ClassFamily == family || (entry.ClassFamily < 0 && found == nullptr)))
            {
                found = &entry;
            }
        }

        return found;
    }

    // The skill whose fix time the harmony option of the weapon lowers, or 0.
    inline int OptionSkill(int group, int number, int harmonyOption, int family)
    {
        const auto* weapon = FindWeapon(group, number, family);
        if (weapon == nullptr)
        {
            return 0;
        }

        for (int i = 0; i < 3; ++i)
        {
            if (harmonyOption == OptionNumbers[i])
            {
                return weapon->Skills[i];
            }
        }

        return 0;
    }

    // The share by which the option lowers the fix time at a harmony level (x0.6 for the elf).
    inline float OptionCutOf(int harmonyLevel, int family)
    {
        const int level = std::clamp(harmonyLevel, 0, static_cast<int>(std::size(OptionCut)) - 1);
        return OptionCut[level] * (family == 8 ? ElfOptionFactor : 1.f);
    }
}
