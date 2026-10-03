#pragma once

#include <cstdint>
#include <span>

namespace GameLogic::Items::KundunSymbols
{
    // Symbols of Kundun are a currency of this server (plugin "Symbols of Kundun currency"):
    // picked up symbols go into a counter of the character instead of the inventory, and the
    // symbol shop npc (Delgado) sells for symbols instead of zen. The server sends the balance
    // and, when a merchant store opens, the symbol prices of its slots (none for a zen shop).
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
    bool IsSymbolShopOpen();
    bool TryGetShopPrice(int slot, std::uint32_t& price);
}
