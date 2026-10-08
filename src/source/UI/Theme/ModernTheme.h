#pragma once

#include <string>

namespace UI::Theme
{
    // The modern look of the windows, switched together with the bottom HUD (GameConfig ModernHud: Options ->
    // Modern HUD; read at the start, so the windows change at the next start while the HUD changes at once).
    // A texture of the interface with a file of the same name in Data\Interface\ModernUI\ is loaded from there:
    // the windows keep their code and layout, only their textures change (tools/hud/modern_ui.py). The bottom HUD
    // has its own folder (Interface\Modern, CNewUIMainFrameWindow::LoadImages).
    bool IsModern();

    // The path LoadBitmap should open for fullPath ("Data\Interface\...\name.tga"): the modern file when the theme is
    // on and the file exists, otherwise fullPath itself.
    std::wstring ResolveTexturePath(const std::wstring& fullPath);
}
