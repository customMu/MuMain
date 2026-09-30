#pragma once

#include <cmath>

namespace GameLogic::Items
{
    // Armor defense by set rank. Must match the server data (armor-ranks-defense.sql, generated from
    // mu-set-ranks.xlsx): the base defense of every armor piece and its level bonus.
    // Level bonus = base x (4% per level up to +9, then 6/6/8/8/10/10%) x (1 + 3% x (rank - 1)),
    // rounded half up — the server stores the same rounded values per level.
    struct ArmorRank
    {
        int Type;
        int BaseDefense;
        int Rank;
    };

    inline constexpr ArmorRank ArmorRanks[] =
    {
        { ITEM_HELM + 2, 39, 1 }, // Pad Helm
        { ITEM_ARMOR + 2, 56, 1 }, // Pad Armor
        { ITEM_PANTS + 2, 48, 1 }, // Pad Pants
        { ITEM_GLOVES + 2, 35, 1 }, // Pad Gloves
        { ITEM_BOOTS + 2, 39, 1 }, // Pad Boots
        { ITEM_HELM + 5, 39, 1 }, // Leather Helm
        { ITEM_ARMOR + 5, 56, 1 }, // Leather Armor
        { ITEM_PANTS + 5, 48, 1 }, // Leather Pants
        { ITEM_GLOVES + 5, 35, 1 }, // Leather Gloves
        { ITEM_BOOTS + 5, 39, 1 }, // Leather Boots
        { ITEM_HELM + 10, 39, 1 }, // Vine Helm
        { ITEM_ARMOR + 10, 56, 1 }, // Vine Armor
        { ITEM_PANTS + 10, 48, 1 }, // Vine Pants
        { ITEM_GLOVES + 10, 35, 1 }, // Vine Gloves
        { ITEM_BOOTS + 10, 39, 1 }, // Vine Boots
        { ITEM_HELM + 39, 100, 1 }, // Mistery Helm
        { ITEM_ARMOR + 39, 145, 1 }, // Mistery Armor
        { ITEM_PANTS + 39, 123, 1 }, // Mistery Pants
        { ITEM_GLOVES + 39, 89, 1 }, // Mistery Gloves
        { ITEM_BOOTS + 39, 100, 1 }, // Mistery Boots
        { ITEM_HELM + 4, 93, 2 }, // Bone Helm
        { ITEM_ARMOR + 4, 134, 2 }, // Bone Armor
        { ITEM_PANTS + 4, 114, 2 }, // Bone Pants
        { ITEM_GLOVES + 4, 83, 2 }, // Bone Gloves
        { ITEM_BOOTS + 4, 93, 2 }, // Bone Boots
        { ITEM_HELM + 6, 93, 2 }, // Scale Helm
        { ITEM_ARMOR + 6, 134, 2 }, // Scale Armor
        { ITEM_PANTS + 6, 114, 2 }, // Scale Pants
        { ITEM_GLOVES + 6, 83, 2 }, // Scale Gloves
        { ITEM_BOOTS + 6, 93, 2 }, // Scale Boots
        { ITEM_HELM + 11, 93, 2 }, // Silk Helm
        { ITEM_ARMOR + 11, 134, 2 }, // Silk Armor
        { ITEM_PANTS + 11, 114, 2 }, // Silk Pants
        { ITEM_GLOVES + 11, 83, 2 }, // Silk Gloves
        { ITEM_BOOTS + 11, 93, 2 }, // Silk Boots
        { ITEM_HELM + 7, 123, 3 }, // Sphinx Mask
        { ITEM_ARMOR + 7, 177, 3 }, // Sphinx Armor
        { ITEM_PANTS + 7, 150, 3 }, // Sphinx Pants
        { ITEM_GLOVES + 7, 109, 3 }, // Sphinx Gloves
        { ITEM_BOOTS + 7, 123, 3 }, // Sphinx Boots
        { ITEM_HELM + 8, 123, 3 }, // Brass Helm
        { ITEM_ARMOR + 8, 177, 3 }, // Brass Armor
        { ITEM_PANTS + 8, 150, 3 }, // Brass Pants
        { ITEM_GLOVES + 8, 109, 3 }, // Brass Gloves
        { ITEM_BOOTS + 8, 123, 3 }, // Brass Boots
        { ITEM_HELM + 12, 123, 3 }, // Wind Helm
        { ITEM_ARMOR + 12, 177, 3 }, // Wind Armor
        { ITEM_PANTS + 12, 150, 3 }, // Wind Pants
        { ITEM_GLOVES + 12, 109, 3 }, // Wind Gloves
        { ITEM_BOOTS + 12, 123, 3 }, // Wind Boots
        { ITEM_HELM + 3, 175, 4 }, // Legendary Helm
        { ITEM_ARMOR + 3, 252, 4 }, // Legendary Armor
        { ITEM_PANTS + 3, 213, 4 }, // Legendary Pants
        { ITEM_GLOVES + 3, 155, 4 }, // Legendary Gloves
        { ITEM_BOOTS + 3, 175, 4 }, // Legendary Boots
        { ITEM_HELM + 9, 175, 4 }, // Plate Helm
        { ITEM_ARMOR + 9, 252, 4 }, // Plate Armor
        { ITEM_PANTS + 9, 213, 4 }, // Plate Pants
        { ITEM_GLOVES + 9, 155, 4 }, // Plate Gloves
        { ITEM_BOOTS + 9, 175, 4 }, // Plate Boots
        { ITEM_HELM + 13, 175, 4 }, // Spirit Helm
        { ITEM_ARMOR + 13, 252, 4 }, // Spirit Armor
        { ITEM_PANTS + 13, 213, 4 }, // Spirit Pants
        { ITEM_GLOVES + 13, 155, 4 }, // Spirit Gloves
        { ITEM_BOOTS + 13, 175, 4 }, // Spirit Boots
        { ITEM_HELM + 25, 175, 4 }, // Light Plate Mask
        { ITEM_ARMOR + 25, 252, 4 }, // Light Plate Armor
        { ITEM_PANTS + 25, 213, 4 }, // Light Plate Pants
        { ITEM_GLOVES + 25, 155, 4 }, // Light Plate Gloves
        { ITEM_BOOTS + 25, 175, 4 }, // Light Plate Boots
        { ITEM_HELM + 40, 175, 4 }, // Red Wing Helm
        { ITEM_ARMOR + 40, 252, 4 }, // Red Wing Armor
        { ITEM_PANTS + 40, 213, 4 }, // Red Wing Pants
        { ITEM_GLOVES + 40, 155, 4 }, // Red Wing Gloves
        { ITEM_BOOTS + 40, 175, 4 }, // Red Wing Boots
        { ITEM_HELM + 35, 258, 5 }, // Eclipse Helm
        { ITEM_ARMOR + 35, 373, 5 }, // Eclipse Armor
        { ITEM_PANTS + 35, 316, 5 }, // Eclipse Pants
        { ITEM_GLOVES + 35, 230, 5 }, // Eclipse Gloves
        { ITEM_BOOTS + 35, 258, 5 }, // Eclipse Boots
        { ITEM_HELM + 1, 232, 5 }, // Dragon Helm
        { ITEM_ARMOR + 1, 336, 5 }, // Dragon Armor
        { ITEM_PANTS + 1, 284, 5 }, // Dragon Pants
        { ITEM_GLOVES + 1, 207, 5 }, // Dragon Gloves
        { ITEM_BOOTS + 1, 232, 5 }, // Dragon Boots
        { ITEM_HELM + 34, 284, 5 }, // Ashcrow Helm
        { ITEM_ARMOR + 34, 410, 5 }, // Ashcrow Armor
        { ITEM_PANTS + 34, 347, 5 }, // Ashcrow Pants
        { ITEM_GLOVES + 34, 252, 5 }, // Ashcrow Gloves
        { ITEM_BOOTS + 34, 284, 5 }, // Ashcrow Boots
        { ITEM_HELM + 14, 232, 5 }, // Guardian Helm
        { ITEM_ARMOR + 14, 336, 5 }, // Guardian Armor
        { ITEM_PANTS + 14, 284, 5 }, // Guardian Pants
        { ITEM_GLOVES + 14, 207, 5 }, // Guardian Gloves
        { ITEM_BOOTS + 14, 232, 5 }, // Guardian Boots
        { ITEM_HELM + 36, 284, 5 }, // Iris Helm
        { ITEM_ARMOR + 36, 410, 5 }, // Iris Armor
        { ITEM_PANTS + 36, 347, 5 }, // Iris Pants
        { ITEM_GLOVES + 36, 252, 5 }, // Iris Gloves
        { ITEM_BOOTS + 36, 284, 5 }, // Iris Boots
        { ITEM_HELM + 26, 258, 5 }, // Adamantine Mask
        { ITEM_ARMOR + 26, 373, 5 }, // Adamantine Armor
        { ITEM_PANTS + 26, 316, 5 }, // Adamantine Pants
        { ITEM_GLOVES + 26, 230, 5 }, // Adamantine Gloves
        { ITEM_BOOTS + 26, 258, 5 }, // Adamantine Boots
        { ITEM_HELM + 41, 258, 5 }, // Ancient Helm
        { ITEM_ARMOR + 41, 373, 5 }, // Ancient Armor
        { ITEM_PANTS + 41, 316, 5 }, // Ancient Pants
        { ITEM_GLOVES + 41, 230, 5 }, // Ancient Gloves
        { ITEM_BOOTS + 41, 258, 5 }, // Ancient Boots
        { ITEM_HELM + 59, 307, 5 }, // Sacred Helm
        { ITEM_ARMOR + 59, 444, 5 }, // Sacred Armor
        { ITEM_PANTS + 59, 376, 5 }, // Sacred Pants
        { ITEM_BOOTS + 59, 307, 5 }, // Sacred Boots
        { ITEM_HELM + 18, 386, 6 }, // Grand Soul Helm
        { ITEM_ARMOR + 18, 557, 6 }, // Grand Soul Armor
        { ITEM_PANTS + 18, 471, 6 }, // Grand Soul Pants
        { ITEM_GLOVES + 18, 343, 6 }, // Grand Soul Gloves
        { ITEM_BOOTS + 18, 386, 6 }, // Grand Soul Boots
        { ITEM_HELM + 16, 386, 6 }, // Black Dragon Helm
        { ITEM_ARMOR + 16, 557, 6 }, // Black Dragon Armor
        { ITEM_PANTS + 16, 471, 6 }, // Black Dragon Pants
        { ITEM_GLOVES + 16, 343, 6 }, // Black Dragon Gloves
        { ITEM_BOOTS + 16, 386, 6 }, // Black Dragon Boots
        { ITEM_HELM + 19, 386, 6 }, // Divine Helm
        { ITEM_ARMOR + 19, 557, 6 }, // Divine Armor
        { ITEM_PANTS + 19, 471, 6 }, // Divine Pants
        { ITEM_GLOVES + 19, 343, 6 }, // Divine Gloves
        { ITEM_BOOTS + 19, 386, 6 }, // Divine Boots
        { ITEM_ARMOR + 15, 679, 6 }, // Storm Crow Armor
        { ITEM_PANTS + 15, 575, 6 }, // Storm Crow Pants
        { ITEM_GLOVES + 15, 418, 6 }, // Storm Crow Gloves
        { ITEM_BOOTS + 15, 470, 6 }, // Storm Crow Boots
        { ITEM_HELM + 27, 386, 6 }, // Dark Steel Mask
        { ITEM_ARMOR + 27, 557, 6 }, // Dark Steel Armor
        { ITEM_PANTS + 27, 471, 6 }, // Dark Steel Pants
        { ITEM_GLOVES + 27, 343, 6 }, // Dark Steel Gloves
        { ITEM_BOOTS + 27, 386, 6 }, // Dark Steel Boots
        { ITEM_HELM + 42, 386, 6 }, // Black Rose Helm
        { ITEM_ARMOR + 42, 557, 6 }, // Black Rose Armor
        { ITEM_PANTS + 42, 471, 6 }, // Black Rose Pants
        { ITEM_GLOVES + 42, 343, 6 }, // Black Rose Gloves
        { ITEM_BOOTS + 42, 386, 6 }, // Black Rose Boots
        { ITEM_HELM + 60, 459, 6 }, // Storm Hard Helm
        { ITEM_ARMOR + 60, 663, 6 }, // Storm Hard Armor
        { ITEM_PANTS + 60, 561, 6 }, // Storm Hard Pants
        { ITEM_BOOTS + 60, 459, 6 }, // Storm Hard Boots
        { ITEM_HELM + 22, 539, 7 }, // Dark Soul Helm
        { ITEM_ARMOR + 22, 778, 7 }, // Dark Soul Armor
        { ITEM_PANTS + 22, 658, 7 }, // Dark Soul Pants
        { ITEM_GLOVES + 22, 479, 7 }, // Dark Soul Gloves
        { ITEM_BOOTS + 22, 539, 7 }, // Dark Soul Boots
        { ITEM_HELM + 17, 485, 7 }, // Dark Phoenix Helm
        { ITEM_ARMOR + 17, 700, 7 }, // Dark Phoenix Armor
        { ITEM_PANTS + 17, 593, 7 }, // Dark Phoenix Pants
        { ITEM_GLOVES + 17, 431, 7 }, // Dark Phoenix Gloves
        { ITEM_BOOTS + 17, 485, 7 }, // Dark Phoenix Boots
        { ITEM_HELM + 21, 593, 7 }, // Great Dragon Helm
        { ITEM_ARMOR + 21, 856, 7 }, // Great Dragon Armor
        { ITEM_PANTS + 21, 724, 7 }, // Great Dragon Pants
        { ITEM_GLOVES + 21, 527, 7 }, // Great Dragon Gloves
        { ITEM_BOOTS + 21, 593, 7 }, // Great Dragon Boots
        { ITEM_HELM + 24, 539, 7 }, // Red Spirit Helm
        { ITEM_ARMOR + 24, 778, 7 }, // Red Sprit Armor
        { ITEM_PANTS + 24, 658, 7 }, // Red Spirit Pants
        { ITEM_GLOVES + 24, 479, 7 }, // Red Spirit Gloves
        { ITEM_BOOTS + 24, 539, 7 }, // Red Spirit Boots
        { ITEM_ARMOR + 37, 854, 7 }, // Valiant Armor
        { ITEM_PANTS + 37, 723, 7 }, // Valiant Pants
        { ITEM_GLOVES + 37, 526, 7 }, // Valiant Gloves
        { ITEM_BOOTS + 37, 591, 7 }, // Valiant Boots
        { ITEM_ARMOR + 20, 949, 7 }, // Thunder Hawk Armor
        { ITEM_PANTS + 20, 803, 7 }, // Thunder Hawk Pants
        { ITEM_GLOVES + 20, 584, 7 }, // Thunder Hawk Gloves
        { ITEM_BOOTS + 20, 657, 7 }, // Thunder Hawk Boots
        { ITEM_ARMOR + 23, 1044, 7 }, // Hurricane Armor
        { ITEM_PANTS + 23, 883, 7 }, // Hurricane Pants
        { ITEM_GLOVES + 23, 642, 7 }, // Hurricane Gloves
        { ITEM_BOOTS + 23, 723, 7 }, // Hurricane Boots
        { ITEM_HELM + 38, 485, 7 }, // Glorious Mask
        { ITEM_ARMOR + 38, 700, 7 }, // Glorious Armor
        { ITEM_PANTS + 38, 593, 7 }, // Glorious Pants
        { ITEM_GLOVES + 38, 431, 7 }, // Glorious Gloves
        { ITEM_BOOTS + 38, 485, 7 }, // Glorious Boots
        { ITEM_HELM + 28, 593, 7 }, // Dark Master Mask
        { ITEM_ARMOR + 28, 856, 7 }, // Dark Master Armor
        { ITEM_PANTS + 28, 724, 7 }, // Dark Master Pants
        { ITEM_GLOVES + 28, 527, 7 }, // Dark Master Gloves
        { ITEM_BOOTS + 28, 593, 7 }, // Dark Master Boots
        { ITEM_HELM + 44, 539, 7 }, // Lilium Helm
        { ITEM_ARMOR + 44, 778, 7 }, // Lilium Armor
        { ITEM_PANTS + 44, 658, 7 }, // Lilium Pants
        { ITEM_GLOVES + 44, 479, 7 }, // Lilium Gloves
        { ITEM_BOOTS + 44, 539, 7 }, // Lilium Boots
        { ITEM_HELM + 61, 641, 7 }, // Piercing Helm
        { ITEM_ARMOR + 61, 926, 7 }, // Piercing Armor
        { ITEM_PANTS + 61, 784, 7 }, // Piercing Pants
        { ITEM_BOOTS + 61, 641, 7 }, // Piercing Boots
        { ITEM_HELM + 30, 756, 8 }, // Venom Mist Helm
        { ITEM_ARMOR + 30, 1092, 8 }, // Venom Mist Armor
        { ITEM_PANTS + 30, 924, 8 }, // Venom Mist Pants
        { ITEM_GLOVES + 30, 672, 8 }, // Venom Mist Gloves
        { ITEM_BOOTS + 30, 756, 8 }, // Venom Mist Boots
        { ITEM_HELM + 29, 756, 8 }, // Dragon Knight Helm
        { ITEM_ARMOR + 29, 1092, 8 }, // Dragon Knight Armor
        { ITEM_PANTS + 29, 924, 8 }, // Dragon Knight Pants
        { ITEM_GLOVES + 29, 672, 8 }, // Dragon Knight Gloves
        { ITEM_BOOTS + 29, 756, 8 }, // Dragon Knight Boots
        { ITEM_HELM + 31, 756, 8 }, // Sylphid Ray Helm
        { ITEM_ARMOR + 31, 1092, 8 }, // Sylphid Ray Armor
        { ITEM_PANTS + 31, 924, 8 }, // Sylphid Ray Pants
        { ITEM_GLOVES + 31, 672, 8 }, // Sylphid Ray Gloves
        { ITEM_BOOTS + 31, 756, 8 }, // Sylphid Ray Boots
        { ITEM_ARMOR + 32, 1332, 8 }, // Volcano Armor
        { ITEM_PANTS + 32, 1127, 8 }, // Volcano Pants
        { ITEM_GLOVES + 32, 820, 8 }, // Volcano Gloves
        { ITEM_BOOTS + 32, 922, 8 }, // Volcano Boots
        { ITEM_HELM + 33, 756, 8 }, // Sunlight Mask
        { ITEM_ARMOR + 33, 1092, 8 }, // Sunlight Armor
        { ITEM_PANTS + 33, 924, 8 }, // Sunlight Pants
        { ITEM_GLOVES + 33, 672, 8 }, // Sunlight Gloves
        { ITEM_BOOTS + 33, 756, 8 }, // Sunlight Boots
        { ITEM_HELM + 43, 756, 8 }, // Aura Helm
        { ITEM_ARMOR + 43, 1092, 8 }, // Aura Armor
        { ITEM_PANTS + 43, 924, 8 }, // Aura Pants
        { ITEM_GLOVES + 43, 672, 8 }, // Aura Gloves
        { ITEM_BOOTS + 43, 756, 8 }, // Aura Boots
        { ITEM_HELM + 73, 900, 8 }, // Phoenix Soul Helmet
        { ITEM_ARMOR + 73, 1300, 8 }, // Phoenix Soul Armor
        { ITEM_PANTS + 73, 1100, 8 }, // Phoenix Soul Pants
        { ITEM_BOOTS + 73, 900, 8 }, // Phoenix Soul Boots
    };

    inline const ArmorRank* FindArmorRank(const int itemType)
    {
        for (const auto& armor : ArmorRanks)
        {
            if (armor.Type == itemType)
            {
                return &armor;
            }
        }

        return nullptr;
    }

    inline int ArmorRankLevelBonus(const ArmorRank& armor, const int itemLevel)
    {
        static constexpr int CumulativePercent[16] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 42, 48, 56, 64, 74, 84 };
        const int level = itemLevel < 0 ? 0 : (itemLevel > 15 ? 15 : itemLevel);
        const double bonus = armor.BaseDefense * CumulativePercent[level] / 100.0 * (1.0 + 0.03 * (armor.Rank - 1));
        return static_cast<int>(std::floor(bonus + 0.5));
    }
}
