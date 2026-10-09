#pragma once

#include <algorithm>
#include <iterator>

#include "GameLogic/Combat/SkillCastTime.h"

namespace GameLogic::Combat::SkillCastTime
{
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

    // Whether the item takes the Jewel of Harmony: a rank 7-8 weapon with the fix options (any class).
    inline bool IsOptionWeapon(int group, int number)
    {
        for (const auto& entry : Weapons)
        {
            if (entry.Group == group && entry.Number == number)
            {
                return true;
            }
        }

        return false;
    }

    // The skill whose fix time the skill fix option of the weapon lowers, or 0.
    inline int OptionSkill(int group, int number, int option, int family)
    {
        const auto* weapon = FindWeapon(group, number, family);
        if (weapon == nullptr)
        {
            return 0;
        }

        for (int i = 0; i < 3; ++i)
        {
            if (option == OptionNumbers[i])
            {
                return weapon->Skills[i];
            }
        }

        return 0;
    }

    // The illusion options of the rank 7-8 weapons (Illusion of Noria): option = 11 + 3 x effect + the index of the skill
    // on the weapon; an Echo gives a random effect for its skill. Only the weapon in slot 0 counts, each option only for
    // its skill. Must match the server (IllusionWeaponOptions) and tools/balance/illusion_extra_options.py.
    enum class IllusionEffect
    {
        None = -1,
        Haste = 0,    // the cast time of the skill (OptionCut)
        Vampiric = 1, // health from the damage of the skill: 5 % at level 10
        Siphon = 2,   // mana from the damage of the skill: 5 %
        Fury = 3,     // the chance of a double hit of the skill: 10 %
    };

    inline IllusionEffect EffectOf(int option)
    {
        const int offset = option - OptionNumbers[0];
        return offset >= 0 && offset < 12 ? static_cast<IllusionEffect>(offset / 3) : IllusionEffect::None;
    }

    // The skill of an illusion option of any effect on the weapon for the class, or 0.
    inline int IllusionOptionSkill(int group, int number, int option, int family)
    {
        const int offset = option - OptionNumbers[0];
        if (offset < 0 || offset >= 12)
        {
            return 0;
        }

        const auto* weapon = FindWeapon(group, number, family);
        return weapon != nullptr ? weapon->Skills[offset % 3] : 0;
    }

    // The value of a Vampiric / Siphon / Fury option at its level, e.g. 0.05 for 5 % (the curve of the cast time cut).
    inline float ExtraOptionValue(int option, int level)
    {
        const float maximum = EffectOf(option) == IllusionEffect::Fury ? 0.10f : 0.05f;
        const int index = std::clamp(level, 0, static_cast<int>(std::size(OptionCut)) - 1);
        return OptionCut[index] / OptionCut[std::size(OptionCut) - 1] * maximum;
    }

    // The share by which the skill fix option lowers the fix time at its level (x0.6 for the elf).
    inline float OptionCutOf(int harmonyLevel, int family)
    {
        const int level = std::clamp(harmonyLevel, 0, static_cast<int>(std::size(OptionCut)) - 1);
        return OptionCut[level] * (family == 8 ? ElfOptionFactor : 1.f);
    }

    // The time of one cast of the skill like the server plugin "Skill cast time" (SkillCastTimePlugIn.GetCastTime):
    // the fix time, below the fix speed the animation at the attack / magic speed of the hero, both cut by the skill fix
    // options (cut; no speed above the fix is needed for them), not below the floor. 0 when the skill has no fix time.
    inline float CastSeconds(int skill, float attackSpeed, float magicSpeed, float cut)
    {
        for (const auto& entry : SkillTimes)
        {
            if (entry.Skill != skill)
            {
                continue;
            }

            const float fix = entry.FixMilliseconds / 1000.f;
            const float keep = 1.f - std::clamp(cut, 0.f, 0.9f);
            float seconds = fix;
            if (entry.Speed != SpeedStat::None)
            {
                const float curve = SpeedCurve(entry.Speed, entry.Speed == SpeedStat::Attack ? attackSpeed : magicSpeed);
                if (curve > 0.f)
                {
                    seconds = std::max(seconds, fix / curve);
                }
            }

            return std::max(CastFloorSeconds, seconds * keep);
        }

        return 0.f;
    }
}
