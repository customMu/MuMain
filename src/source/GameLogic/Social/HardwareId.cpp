#include "stdafx.h"
#include "GameLogic/Social/HardwareId.h"

#include <string>

namespace GameLogic::Social::HardwareId
{
    namespace
    {
        std::wstring ReadMachineGuid()
        {
#ifdef _WIN32
            wchar_t value[64] = {};
            DWORD size = sizeof(value);
            if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid",
                             RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, nullptr, value, &size) == ERROR_SUCCESS)
            {
                return value;
            }
#endif
            return {};
        }

        std::wstring ReadSystemDriveSerial()
        {
#ifdef _WIN32
            wchar_t windows[MAX_PATH] = {};
            if (GetWindowsDirectoryW(windows, MAX_PATH) >= 3)
            {
                const wchar_t root[4] = { windows[0], L':', L'\\', 0 };
                DWORD serial = 0;
                if (GetVolumeInformationW(root, nullptr, 0, &serial, nullptr, nullptr, nullptr, 0))
                {
                    return std::to_wstring(serial);
                }
            }
#endif
            return {};
        }

        // 64 bit FNV-1a; two runs with different starts give the 16 bytes
        std::uint64_t Fnv1a(const std::wstring& text, std::uint64_t hash)
        {
            for (const wchar_t c : text)
            {
                hash ^= static_cast<std::uint64_t>(c & 0xFF);
                hash *= 0x100000001B3ull;
                hash ^= static_cast<std::uint64_t>((c >> 8) & 0xFF);
                hash *= 0x100000001B3ull;
            }

            return hash;
        }
    }

    const std::array<std::uint8_t, 16>& Get()
    {
        static const std::array<std::uint8_t, 16> id = []
        {
            const std::wstring source = L"MU-HWID-1|" + ReadMachineGuid() + L"|" + ReadSystemDriveSerial();
            const std::uint64_t first = Fnv1a(source, 0xCBF29CE484222325ull);
            const std::uint64_t second = Fnv1a(source + L"|2", first ^ 0x9E3779B97F4A7C15ull);
            std::array<std::uint8_t, 16> bytes{};
            for (int i = 0; i < 8; ++i)
            {
                bytes[i] = static_cast<std::uint8_t>(first >> (8 * i));
                bytes[8 + i] = static_cast<std::uint8_t>(second >> (8 * i));
            }

            return bytes;
        }();
        return id;
    }
}
