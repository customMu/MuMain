#include "stdafx.h"
#include "GameLogic/Items/KundunSymbols.h"

#include <array>

namespace GameLogic::Items::KundunSymbols
{
    namespace
    {
        constexpr std::uint32_t NotForSale = 0;

        std::uint32_t g_Balance = 0;
        bool g_IsBalanceKnown = false;
        bool g_IsSymbolShopOpen = false;
        std::array<std::uint32_t, MaximumShopSlots> g_ShopPrices{};
    }

    void SetBalance(const std::uint32_t balance)
    {
        g_Balance = balance;
        g_IsBalanceKnown = true;
    }

    std::uint32_t GetBalance()
    {
        return g_Balance;
    }

    bool IsBalanceKnown()
    {
        return g_IsBalanceKnown;
    }

    void SetShopPrices(const std::span<const ShopPrice> prices)
    {
        g_ShopPrices.fill(NotForSale);
        g_IsSymbolShopOpen = !prices.empty();
        for (const auto& entry : prices)
        {
            if (entry.slot < MaximumShopSlots)
            {
                g_ShopPrices[entry.slot] = entry.price;
            }
        }
    }

    bool IsSymbolShopOpen()
    {
        return g_IsSymbolShopOpen;
    }

    bool TryGetShopPrice(const int slot, std::uint32_t& price)
    {
        if (!g_IsSymbolShopOpen || slot < 0 || slot >= MaximumShopSlots)
        {
            return false;
        }

        price = g_ShopPrices[slot];
        return true;
    }
}
