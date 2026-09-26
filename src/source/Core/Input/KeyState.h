#pragma once

namespace Core::Input
{
    void RecordLeftMouseButtonPressEdge();
    void ClearLeftMouseButtonPressEdge();

    // Returns false if a left mouse button press arrived sooner than kMinLeftPressIntervalMs after the
    // previously accepted press. Such presses must be ignored completely (no Push/held state), which
    // limits the click rate everywhere (windows, buttons, the game world) independent of the frame rate.
    bool AcceptLeftMousePress();

    // Portable replacement for the Win32 "is this key currently down" check
    // (HIBYTE(GetAsyncKeyState(vk)) == 128 / & 0x8000), backed by SDL keyboard
    // state. The argument is a Win32 virtual-key code (VK_*) or an ASCII letter
    // or digit, matching what the existing call sites already pass.
    bool IsKeyDown(int virtualKey);
}
