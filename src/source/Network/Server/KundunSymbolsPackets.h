#pragma once

#include <cstdint>
#include <span>

namespace Network::Server::KundunSymbols
{
    // Custom packets of this server (OpenMU: KundunSymbolsViewPlugIn), head code 0xFB:
    //   C1 08 FB 01 [balance: uint32 little endian]                         - Symbols of Kundun balance
    //   C2 [size: uint16 big endian] FB 02 [count] count x [slot, price: uint32 little endian]
    //                                                                       - prices of the opened symbol shop
    inline constexpr std::uint8_t HeadCode = 0xFB;

    void ReceivePacket(std::span<const std::uint8_t> packet);
}
