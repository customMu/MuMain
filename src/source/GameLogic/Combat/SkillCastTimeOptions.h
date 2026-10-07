#pragma once

#include <algorithm>
#include <iterator>

#include "GameLogic/Combat/SkillCastTime.h"

namespace GameLogic::Combat::SkillCastTime
{
    // The stat an attack action follows (None: a fixed play speed).
    enum class SpeedStat
    {
        None,
        Attack,
        Magic,
    };

    // One speed curve for all actions: play speed at the fix x (offset + speed) / (offset + speed at the fix). The
    // offsets are equal in agility (attack speed 0.03, magic speed 0.025 per agility + 35 of a median weapon):
    // (181 + 35) / 0.03 = (145 + 35) / 0.025 = 7 200 - at the same agility every action is as much slower than its fix.
    // Must match the server plugin "Skill cast time" (AttackSpeedCurveOffset, MagicSpeedCurveOffset).
    inline constexpr float AttackSpeedCurveOffset = 181.0f;
    inline constexpr float MagicSpeedCurveOffset = 145.0f;

    // The factor of the play speed at the fix for a speed of the hero (1 at the fix speed, more above it).
    inline float SpeedCurve(SpeedStat stat, float speed)
    {
        if (stat == SpeedStat::None)
        {
            return 1.f;
        }

        const float offset = stat == SpeedStat::Attack ? AttackSpeedCurveOffset : MagicSpeedCurveOffset;
        const float atFix = stat == SpeedStat::Attack ? AttackSpeedAtFix : MagicSpeedAtFix;
        return (offset + std::max(0.f, speed)) / (offset + atFix);
    }

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
