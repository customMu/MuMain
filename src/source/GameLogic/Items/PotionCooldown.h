#pragma once

#include <cstdint>

namespace GameLogic::Items
{
    // Potion cooldown, the same as the server (potion plugins: CooldownTime 5 s, health and mana separately;
    // session 9, class-balance-potions.sql). Used for the cooldown shown on the item hotkeys and by the MU Helper.
    inline constexpr std::uint64_t PotionCooldownMilliseconds = 5000;

    inline std::uint64_t g_HealthPotionUsedAt = 0;
    inline std::uint64_t g_ManaPotionUsedAt = 0;

    inline bool IsHealthPotion(const int type)
    {
        return (type >= ITEM_SMALL_HEALING_POTION && type <= ITEM_LARGE_HEALING_POTION)
            || (type >= ITEM_POTION + 38 && type <= ITEM_POTION + 40);   // complex potions use the health cooldown
    }

    inline bool IsManaPotion(const int type)
    {
        return type >= ITEM_SMALL_MANA_POTION && type <= ITEM_LARGE_MANA_POTION;
    }

    // What a potion recovers and from which reset it can be used, the same as the server
    // (potion plugins: AdditionalRecover + TotalRecoverPercentage; potion-resets.sql; plugin "Account bound items").
    struct PotionInfo
    {
        int Flat;
        int Percent;
        int RequiredResets;
        bool AccountBound;
    };

    inline const PotionInfo* GetPotionInfo(const int type)
    {
        static constexpr PotionInfo Small{ 100, 5, 0, false };
        static constexpr PotionInfo Medium{ 300, 10, 5, false };
        static constexpr PotionInfo Large{ 1000, 15, 10, true };
        switch (type)
        {
        case ITEM_SMALL_HEALING_POTION:
        case ITEM_SMALL_MANA_POTION:
            return &Small;
        case ITEM_MEDIUM_HEALING_POTION:
        case ITEM_MEDIUM_MANA_POTION:
            return &Medium;
        case ITEM_LARGE_HEALING_POTION:
        case ITEM_LARGE_MANA_POTION:
            return &Large;
        default:
            return nullptr;
        }
    }

    // False for a potion of a later reset step; other items are always usable here.
    inline bool CanUsePotion(const int type, const int resets)
    {
        const PotionInfo* info = GetPotionInfo(type);
        return info == nullptr || resets >= info->RequiredResets;
    }

    inline void RecordPotionCooldown(const int type)
    {
        if (IsHealthPotion(type))
        {
            g_HealthPotionUsedAt = GetTickCount64();
        }
        else if (IsManaPotion(type))
        {
            g_ManaPotionUsedAt = GetTickCount64();
        }
    }

    // Remaining part of the cooldown of the potion type: 1 = just used, 0 = ready (also for non-potions).
    inline float GetPotionCooldownFraction(const int type)
    {
        const std::uint64_t usedAt = IsHealthPotion(type) ? g_HealthPotionUsedAt : (IsManaPotion(type) ? g_ManaPotionUsedAt : 0);
        if (usedAt == 0)
        {
            return 0.f;
        }

        const std::uint64_t elapsed = GetTickCount64() - usedAt;
        return elapsed >= PotionCooldownMilliseconds ? 0.f : 1.f - static_cast<float>(elapsed) / PotionCooldownMilliseconds;
    }
}
