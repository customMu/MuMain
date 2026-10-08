#include "stdafx.h"
#include "UI/Theme/ModernTheme.h"

#include <cwctype>
#include <filesystem>

#include "Data/GameConfig/GameConfig.h"

namespace UI::Theme
{
    namespace
    {
        constexpr wchar_t InterfaceFolder[] = L"data\\interface\\";
        constexpr wchar_t ModernFolder[] = L"Data\\Interface\\ModernUI\\";

        std::wstring Lower(std::wstring text)
        {
            for (auto& c : text)
            {
                c = static_cast<wchar_t>(std::towlower(c));
            }
            return text;
        }

        // The file the texture loader opens for a texture name: .jpg -> .OZJ, .tga -> .OZT, .bmp -> .OZB.
        std::wstring PackedName(const std::wstring& path)
        {
            const auto dot = path.find_last_of(L'.');
            if (dot == std::wstring::npos)
            {
                return path;
            }

            const std::wstring ext = Lower(path.substr(dot));
            const wchar_t* packed = ext == L".jpg" ? L".OZJ" : ext == L".tga" ? L".OZT" : ext == L".bmp" ? L".OZB" : nullptr;
            return packed ? path.substr(0, dot) + packed : path;
        }
    }

    bool IsModern()
    {
        // read once: the windows load their textures at the start, so a change applies at the next start
        static const bool modern = GameConfig::GetInstance().GetModernHud();
        return modern;
    }

    std::wstring ResolveTexturePath(const std::wstring& fullPath)
    {
        if (!IsModern())
        {
            return fullPath;
        }

        const std::wstring lower = Lower(fullPath);
        if (lower.rfind(InterfaceFolder, 0) != 0 || lower.find(L"\\modern") != std::wstring::npos)
        {
            return fullPath;
        }

        const auto slash = fullPath.find_last_of(L'\\');
        const std::wstring modern = std::wstring(ModernFolder) + fullPath.substr(slash + 1);
        std::error_code error;
        return std::filesystem::is_regular_file(PackedName(modern), error) ? modern : fullPath;
    }
}
