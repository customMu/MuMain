#pragma once

// The melee skills which need a weapon in hand (09.10.2026): Twisting Slash, Rageful Blow, Death Stab, Strike of
// Destruction, Fire Slash, Power Slash, Spiral Slash, Flame Strike and their master versions. A staff doesn't count
// (as in the original client). The server checks the same list (OpenMU: Player.TryConsumeForSkillAsync,
// SkillWeaponRequirement.cs).

#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Skills/SkillManager.h"

namespace GameLogic::Combat::SkillWeaponRequirement
{
    inline bool NeedsWeapon(const ActionSkillType skill)
    {
        switch (gSkillManager.MasterSkillToBaseSkillIndex(skill))
        {
        case AT_SKILL_TWISTING_SLASH:
        case AT_SKILL_RAGEFUL_BLOW:
        case AT_SKILL_DEATHSTAB:
        case AT_SKILL_STRIKE_OF_DESTRUCTION:
        case AT_SKILL_FIRE_SLASH:
        case AT_SKILL_POWER_SLASH:
        case AT_SKILL_SPIRAL_SLASH:
        case AT_SKILL_FLAME_STRIKE:
            return true;
        default:
            return false;
        }
    }

    // a weapon (swords, axes, maces, spears, bows; no staff, no shield) in one of the hands
    inline bool IsMeleeWeapon(const int type)
    {
        return type >= ITEM_SWORD && type < ITEM_STAFF;
    }

    inline bool HasWeapon()
    {
        return CharacterMachine != nullptr
               && (IsMeleeWeapon(CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type)
                   || IsMeleeWeapon(CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type));
    }

    inline bool CanUse(const ActionSkillType skill)
    {
        return !NeedsWeapon(skill) || HasWeapon();
    }
}
