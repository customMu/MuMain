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
        BuyLesserStone = 0x20, // a Lesser Mirage Stone for Illusion Shards (the shop of the warden)
        BuyWard = 0x21,        // the Veil Ward for Illusion Shards (no damage of the Golden Curse)
        BuyBlessing = 0x22,    // the Blessing of the Veil for Illusion Shards (+50 % damage in the illusion)
        State = 0x30,       // asks for the state (FB 14) after entering a map
    };

    // the group of the entry of the price list which is the Veil Ward (its number is the magic effect)
    inline constexpr int WardExchangeGroup = 0xFF;

    // the magic effect of the Veil Ward (server plugin "Illusion of Noria", WardEffectNumber); not in _enum.h
    inline constexpr int VeilWardEffect = 189;
    // the magic effect of the Blessing of the Veil (server BlessingEffectNumber): more damage to the illusion; it works
    // together with the Veil Ward
    inline constexpr int VeilBlessingEffect = 185;

    // an entry of the price list of the warden: the Lesser Mirage Stone (14/196) or the Veil Ward (group 0xFF) and its
    // price in Illusion Shards
    struct ExchangeEntry
    {
        int Group = 0;
        int Number = 0;
        int Price = 0;
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
    // Jewel of Illusion); a random stone of the warden or of the boss, can be traded. Server: IllusionOfNoriaConfiguration.Echoes, tools/balance/illusion_of_noria.py.
    struct Echo
    {
        int Item;
        int Skill;
        const wchar_t* SkillName; // game names are not translated
        const wchar_t* Model;     // Data\\Item\\<Model>01.bmd: the jewel in the colour of the class (tools/bmd/examples/illusion_stones.py)
    };

    inline constexpr Echo Echoes[] =
    {
        { ITEM_POTION + 173, 41, L"Twisting Slash", L"EchoDK" },
        { ITEM_POTION + 174, 43, L"Death Stab", L"EchoDK" },
        { ITEM_POTION + 175, 232, L"Strike of Destruction", L"EchoDK" },
        { ITEM_POTION + 176, 56, L"Power Slash", L"EchoMG" },
        { ITEM_POTION + 177, 55, L"Fire Slash", L"EchoMG" },
        { ITEM_POTION + 178, 9, L"Evil Spirit", L"EchoDW" },
        { ITEM_POTION + 179, 237, L"Gigantic Storm", L"EchoMG" },
        { ITEM_POTION + 180, 8, L"Twister", L"EchoMG" },
        { ITEM_POTION + 181, 38, L"Decay", L"EchoDW" },
        { ITEM_POTION + 182, 39, L"Ice Storm", L"EchoDW" },
        { ITEM_POTION + 183, 235, L"Multi-Shot", L"EchoElf" },
        { ITEM_POTION + 184, 52, L"Penetration", L"EchoElf" },
        { ITEM_POTION + 185, 46, L"Starfall", L"EchoElf" },
        { ITEM_POTION + 186, 78, L"Fire Scream", L"EchoDL" },
        { ITEM_POTION + 187, 61, L"Fire Burst", L"EchoDL" },
        { ITEM_POTION + 188, 238, L"Chaotic Diseier", L"EchoDL" },
        { ITEM_POTION + 189, 230, L"Lightning Shock", L"EchoSUM" },
        { ITEM_POTION + 190, 215, L"Chain Lightning", L"EchoSUM" },
        { ITEM_POTION + 191, 225, L"Pollution", L"EchoSUM" },
        { ITEM_POTION + 192, 264, L"Dragon Roar", L"EchoRF" },
        { ITEM_POTION + 193, 263, L"Dark Side", L"EchoRF" },
        { ITEM_POTION + 194, 260, L"Killing Blow", L"EchoRF" },
    };

    // The echo of an item, or nullptr.
    const Echo* FindEcho(int itemType);

    // Whether the item is a stone of the skill fix option: Jewel of Illusion, an Echo or a Mirage Stone.
    bool IsSkillFixStone(int itemType);

    // Whether the stone can be used on the target item (the server checks it again): the Jewel of Illusion and the
    // Echoes on a rank 7-8 weapon of +10 without the option (an Echo only if the weapon has its skill for this class),
    // the Mirage Stones on a weapon with the option below the maximum level.
    bool CanApplyStone(int stoneType, const ITEM& target);

    const WardenInfo& GetInfo();

    // Parses FB 12 and opens the dialog of the warden.
    void ReceiveWardenDialog(std::span<const std::uint8_t> packet);

    // Parses FB 14: the same state without the dialog (the mark over the warden, the daily quest in the quest window).
    void ReceiveWardenState(std::span<const std::uint8_t> packet);

    // FB 15 [seconds]: the Golden Curse burns this long from now (renewed every second near the boss).
    void ReceiveCurseTime(int seconds);

    // The seconds the Golden Curse still burns (the timer of the debuff icon), 0 when unknown or over.
    int CurseSecondsLeft();

    // The text of an illusion option of a weapon for its skill, e.g. "Vampiric Twisting Slash: 5.00% of the damage to HP"
    // (Haste, Vampiric, Siphon, Fury - GameLogic/Combat/SkillCastTimeOptions.h); empty for other options.
    void FormatOption(wchar_t* out, size_t size, int option, int level, int family, const wchar_t* skillName);

    // FB 16 [seconds, 2 bytes]: the Veil Ward lasts this long from now.
    void ReceiveWardTime(int seconds);

    // The seconds the Veil Ward still lasts (the timer of its icon), 0 when unknown or over.
    int WardSecondsLeft();

    // FB 17 [seconds, 2 bytes]: the Blessing of the Veil lasts this long from now (the timer of the buff icon).
    void ReceiveBlessingTime(int seconds);

    // The seconds the Blessing of the Veil still lasts, 0 when unknown or over.
    int BlessingSecondsLeft();

    // Whether the state came from the server since the start of the client.
    bool HasState();

    // The daily quest as it is now: 0 can be taken, 1 active, 2 completed, 3 done today (a new day turns 3 into 0 without
    // a new packet).
    int CurrentDailyState();

    // The seconds until the next day of the daily quest, counted down since the last state.
    std::uint32_t SecondsUntilNextDay();

    // What the mark over Warden Eldrin shows: 0 nothing, 1 a quest or the daily quest can be taken (blue !), 2 a reward
    // waits (gold !).
    int WardenMark();

    // Asks the server for the state after a map change and draws the mark over the warden (from CNewUINameWindow::Render).
    void RenderWardenMark();

    // Sends an action of the dialog (C1 05 FB 13 [action]).
    void SendAction(std::uint8_t action);


    // The whistle (14/171) and the new names of the jewels of the harmony options; called after the item file is loaded.
    void RegisterItems();

    // The name of a monster of the illusion (the client has no names for the new numbers), or nullptr.
    const wchar_t* MonsterName(int type);

    inline bool IsIllusionMonster(int type) { return type >= FirstMonster && type <= LastMonster; }
    inline bool IsBoss(int type) { return type == Boss; }
}
