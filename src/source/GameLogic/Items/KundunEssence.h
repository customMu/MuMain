#pragma once

#include <cstdint>
#include <span>

namespace GameLogic::Items::KundunEssence
{
    // The Kundun Essence is a currency of this server (plugin "Kundun Essence currency"): a counter
    // of the character which is earned in the Kalima instance and the chamber of Kundun. The essence
    // shop npc (Delgado) sells for essence instead of zen. The server sends the balance and, when a
    // merchant store opens, the essence prices of its slots (none for a zen shop).
    inline constexpr int MaximumShopSlots = 120;

    struct ShopPrice
    {
        std::uint8_t slot;
        std::uint32_t price;
    };

    void SetBalance(std::uint32_t balance);
    std::uint32_t GetBalance();
    bool IsBalanceKnown();

    // An empty list means the opened shop sells for zen.
    void SetShopPrices(std::span<const ShopPrice> prices);
    bool IsEssenceShopOpen();
    bool TryGetShopPrice(int slot, std::uint32_t& price);
}
