#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace GameLogic::Events::IllusionOfNoria
{
    // The Illusion of Noria (server plugin "Illusion of Noria"): map 82, a copy of Noria (World4) where the monsters of
    // Noria roam as strong illusions (rank 7 weapons, 20 resets). Warden Eldrin (in Noria next to the Chaos Machine and in
    // the town of the illusion) gives the quest of the Whistle of the Veil (Condra, the outcast of Karutan, stole it), lets
    // the character in and out, gives the daily quest and resets the harmony option of the equipped weapons. Every 3 000
    // kills the Gilded Colossus (a big golden Stone Golem) awakens; its Golden Curse burns the players near it.
    inline constexpr int MapNumber = 82;
    inline constexpr int FirstMonster = 710; // Illusion Goblin ... Illusion Elite Goblin (710-717)
    inline constexpr int LastMonster = 717;
    inline constexpr int Boss = 718;         // Gilded Colossus
    inline constexpr int Warden = 719;       // Warden Eldrin
    inline constexpr int CurseEffect = 188;  // Golden Curse (eBuffState)
    inline constexpr int CurseDamagePerSecond = 300;

    // The dialog of the warden: C2 FB 12 from the server, actions C1 05 FB 13 [action] from the client.
    enum Action : std::uint8_t
    {
        AcceptQuest = 1,
        TurnIn = 2,
        Enter = 3,
        TakeDaily = 4,
        ClaimDaily = 5,
        Return = 6,
        ResetOption = 0x10, // + slot (0 left hand, 1 right hand)
        Exchange = 0x20,    // + index of the exchange list
    };

    struct ExchangeEntry
    {
        int Group = 0;
        int Number = 0;
        int Price = 0; // Illusion Shards
    };

    struct DailyMonster
    {
        int Number = 0;
        int Kills = 0;
    };

    struct ResetWeapon
    {
        int Slot = 0;
        int Group = 0;
        int Number = 0;
        int Option = 0;
        int OptionLevel = 0;
        int Rank = 0;
        int Jewels = 0;
        int LesserStones = 0;
        int GreaterStones = 0;
        bool CanPay = false;
    };

    struct WardenInfo
    {
        bool InIllusion = false;
        int QuestState = 0; // 0 not taken, 1 taken, 2 done
        bool HasWhistle = false;
        int Resets = 0;
        int RequiredResets = 0;
        int DailyState = 0; // 0 can be taken, 1 active, 2 completed, 3 done today
        int DailyKillsNeeded = 0;
        std::uint32_t SecondsUntilNextDay = 0;
        std::vector<DailyMonster> Daily;
        std::vector<ResetWeapon> Weapons;
        std::uint32_t Shards = 0;
        std::vector<ExchangeEntry> Exchange;
    };

    // The Echoes (14/173-194): add the harmony option of one skill to a rank 7-8 weapon which has it (75 %, like the
    // Jewel of Illusion); sold by the warden. Server: IllusionOfNoriaConfiguration.Echoes, tools/balance/illusion_of_noria.py.
    struct Echo
    {
        int Item;
        int Skill;
        const wchar_t* SkillName; // game names are not translated
    };

    inline constexpr Echo Echoes[] =
    {
        { ITEM_POTION + 173, 41, L"Twisting Slash" },
        { ITEM_POTION + 174, 43, L"Death Stab" },
        { ITEM_POTION + 175, 232, L"Strike of Destruction" },
        { ITEM_POTION + 176, 56, L"Power Slash" },
        { ITEM_POTION + 177, 55, L"Fire Slash" },
        { ITEM_POTION + 178, 9, L"Evil Spirit" },
        { ITEM_POTION + 179, 237, L"Gigantic Storm" },
        { ITEM_POTION + 180, 8, L"Twister" },
        { ITEM_POTION + 181, 38, L"Decay" },
        { ITEM_POTION + 182, 39, L"Ice Storm" },
        { ITEM_POTION + 183, 235, L"Multi-Shot" },
        { ITEM_POTION + 184, 52, L"Penetration" },
        { ITEM_POTION + 185, 46, L"Starfall" },
        { ITEM_POTION + 186, 78, L"Fire Scream" },
        { ITEM_POTION + 187, 61, L"Fire Burst" },
        { ITEM_POTION + 188, 238, L"Chaotic Diseier" },
        { ITEM_POTION + 189, 230, L"Lightning Shock" },
        { ITEM_POTION + 190, 215, L"Chain Lightning" },
        { ITEM_POTION + 191, 225, L"Pollution" },
        { ITEM_POTION + 192, 264, L"Dragon Roar" },
        { ITEM_POTION + 193, 263, L"Dark Side" },
        { ITEM_POTION + 194, 260, L"Killing Blow" },
    };

    // The echo of an item, or nullptr.
    const Echo* FindEcho(int itemType);

    const WardenInfo& GetInfo();

    // Parses FB 12 and opens the dialog of the warden.
    void ReceiveWardenDialog(std::span<const std::uint8_t> packet);

    // Sends an action of the dialog (C1 05 FB 13 [action]).
    void SendAction(std::uint8_t action);

    // Buys an item of the shop of the warden (C1 08 FB 13 20 [group] [number u16]).
    void SendBuy(int group, int number);

    // The whistle (14/171) and the new names of the jewels of the harmony options; called after the item file is loaded.
    void RegisterItems();

    // The name of a monster of the illusion (the client has no names for the new numbers), or nullptr.
    const wchar_t* MonsterName(int type);

    inline bool IsIllusionMonster(int type) { return type >= FirstMonster && type <= LastMonster; }
    inline bool IsBoss(int type) { return type == Boss; }
}
