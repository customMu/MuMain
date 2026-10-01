#pragma once

namespace GameLogic::Items::SetGuard
{
    // Set Guard: a complete armor set decreases the received damage by a percentage of its rank, growing with the
    // lowest enhancement level of the set. Must match the server plugin "Set Guard" (SetGuardPlugIn / SetGuardConfiguration).
    struct SetInfo
    {
        int Number;          // item number of the pieces in the groups 7 (helm) to 11 (boots)
        int Rank;            // 1..8
        const wchar_t* Name; // set name without the piece name
    };

    inline constexpr SetInfo Sets[] =
    {
        { 2, 1, L"Pad" },
        { 5, 1, L"Leather" },
        { 10, 1, L"Vine" },
        { 39, 1, L"Mistery" },
        { 4, 2, L"Bone" },
        { 6, 2, L"Scale" },
        { 11, 2, L"Silk" },
        { 7, 3, L"Sphinx" },
        { 8, 3, L"Brass" },
        { 12, 3, L"Wind" },
        { 3, 4, L"Legendary" },
        { 9, 4, L"Plate" },
        { 13, 4, L"Spirit" },
        { 25, 4, L"Light Plate" },
        { 40, 4, L"Red Wing" },
        { 35, 5, L"Eclipse" },
        { 1, 5, L"Dragon" },
        { 34, 5, L"Ashcrow" },
        { 14, 5, L"Guardian" },
        { 36, 5, L"Iris" },
        { 26, 5, L"Adamantine" },
        { 41, 5, L"Ancient" },
        { 59, 5, L"Sacred" },
        { 18, 6, L"Grand Soul" },
        { 16, 6, L"Black Dragon" },
        { 19, 6, L"Divine" },
        { 15, 6, L"Storm Crow" },
        { 27, 6, L"Dark Steel" },
        { 42, 6, L"Black Rose" },
        { 60, 6, L"Storm Hard" },
        { 22, 7, L"Dark Soul" },
        { 17, 7, L"Dark Phoenix" },
        { 21, 7, L"Great Dragon" },
        { 24, 7, L"Red Spirit" },
        { 37, 7, L"Valiant" },
        { 20, 7, L"Thunder Hawk" },
        { 23, 7, L"Hurricane" },
        { 38, 7, L"Glorious" },
        { 28, 7, L"Dark Master" },
        { 44, 7, L"Lilium" },
        { 61, 7, L"Piercing" },
        { 30, 8, L"Venom Mist" },
        { 29, 8, L"Dragon Knight" },
        { 31, 8, L"Sylphid Ray" },
        { 32, 8, L"Volcano" },
        { 33, 8, L"Sunlight" },
        { 43, 8, L"Aura" },
        { 73, 8, L"Phoenix Soul" },
    };

    inline constexpr float RankPercent[8] = { 3.0f, 3.3f, 4.2f, 6.3f, 9.7f, 14.9f, 20.7f, 28.6f };
    inline constexpr int StepLevels[5] = { 5, 9, 11, 13, 15 };
    inline constexpr float StepBonus = 0.10f;

    struct State
    {
        const SetInfo* Set = nullptr; // the set of the equipped armor, if all equipped pieces belong to the same set
        int WornParts = 0;
        int RequiredParts = 0;
        int LowestLevel = 0;
        bool IsComplete = false;
        float Percent = 0.f;          // current damage decrease, 0 if the set is not complete
    };

    const SetInfo* FindSet(int itemType);
    int GetRequiredParts(const SetInfo& set);
    float GetPercent(const SetInfo& set, int lowestLevel);
    int GetNextStepLevel(int lowestLevel); // -1 if all steps are reached
    State GetHeroState();

    // Adds the Set Guard lines to the item tooltip (TextList). Returns the new line count.
    int AppendTooltip(ITEM* item, int textNum);
    // Adds the Set Guard lines of the hero's complete set (buff tooltip). Returns the new line count.
    int AppendBuffTooltip(int textNum);
}
