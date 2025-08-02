#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#define VK_USE_PLATFORM_WIN32_KHR

#ifndef NOMINMAX
#    define NOMINMAX
#endif

#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <windows.h>
#include <windowsx.h>

#include <vulkan/vulkan_win32.h>

namespace vsgWin32
{
    class VSG_DECLSPEC KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using VirtualKeyToKeySymbolMap = std::map<uint16_t, vsg::KeySymbol>;

        bool getKeySymbol(WPARAM wParam, LPARAM lParam, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
        {
            uint16_t modifierMask = 0;

            // see https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-message-flags
            WORD vkCode = LOWORD(wParam); // virtual-key code
            WORD keyFlags = HIWORD(lParam);
            WORD scanCode = LOBYTE(keyFlags);                             // scan code
            BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix

            if (isExtendedKey)
                scanCode = MAKEWORD(scanCode, 0xE0);

            uint32_t virtualKey = ::MapVirtualKeyEx(scanCode, MAPVK_VSC_TO_VK_EX, ::GetKeyboardLayout(0));
            auto itr = _vk2vsg.find(virtualKey);

            if (itr == _vk2vsg.end())
            {
                // What ever the code was in lParam should translate to a Virtual Key that we know of in _vk2vsg
                // If we cannot find it, we simply return.
                return false;
            }

            // This is the base-key that was pressed. (i.e., the VSG enum of the physical key pressed).
            keySymbol = itr->second;

            // Look for any modifiers that may be active.
            BYTE keyState[256];
            if (virtualKey == 0 || !::GetKeyboardState(keyState))
            {
                // if virtualKey was undefined or we could not get the keyboard state, simply return.
                return false;
            }

            // If any of the specific left/right modifier keys are active
            // add the side-independent vsg modifier to the modifier Mask
            switch (virtualKey)
            {
            case VK_LSHIFT:
            case VK_RSHIFT:
                modifierMask |= vsg::KeyModifier::MODKEY_Shift;
                break;

            case VK_LCONTROL:
            case VK_RCONTROL:
                modifierMask |= vsg::KeyModifier::MODKEY_Control;
                break;

            case VK_LMENU:
            case VK_RMENU:
                modifierMask |= vsg::KeyModifier::MODKEY_Alt;
                break;

            default:
                virtualKey = static_cast<uint32_t>(wParam);
                break;
            }

            // Check if caps lock or numlock is toggled.
            if (keyState[VK_CAPITAL] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;
            if (keyState[VK_NUMLOCK] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;

            // Check if the modifier keys are down (these are non-toggle keys, so the high-order bit is relevant!)
            // again, vsg only has a side-independent modifier
            if (keyState[VK_LSHIFT] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Shift;
            if (keyState[VK_RSHIFT] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Shift;
            if (keyState[VK_LCONTROL] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Control;
            if (keyState[VK_RCONTROL] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Control;
            if (keyState[VK_LMENU] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Alt;
            if (keyState[VK_RMENU] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Alt;

            // This is the final keyModifier
            keyModifier = static_cast<vsg::KeyModifier>(modifierMask);

            // The actual keystroke is what we get after the ::ToAscii call
            char asciiKey[2];
            int32_t numChars = ::ToAsciiEx(static_cast<UINT>(wParam), scanCode, keyState, reinterpret_cast<WORD*>(asciiKey), 0, ::GetKeyboardLayout(0));
            if (numChars == 1)
            {
                // it is indeed an ascii character. 0-127
                modifiedKeySymbol = static_cast<vsg::KeySymbol>(asciiKey[0]);
            }
            else
            {
                // otherwise treat the modifiedKeySymbol as the same as the keySymbol.
                modifiedKeySymbol = keySymbol;
            }

            return true;
        }

    protected:
        VirtualKeyToKeySymbolMap _vk2vsg;
    };

    inline vsg::ButtonMask getButtonMask(WPARAM wParam)
    {
        auto mask = (wParam & MK_LBUTTON ? vsg::ButtonMask::BUTTON_MASK_1 : 0) | (wParam & MK_MBUTTON ? vsg::ButtonMask::BUTTON_MASK_2 : 0) | (wParam & MK_RBUTTON ? vsg::ButtonMask::BUTTON_MASK_3 : 0) |
                    (wParam & MK_XBUTTON1 ? vsg::ButtonMask::BUTTON_MASK_4 : 0) | (wParam & MK_XBUTTON2 ? vsg::ButtonMask::BUTTON_MASK_5 : 0);
        return static_cast<vsg::ButtonMask>(mask);
    }

    inline uint32_t getButtonDownEventDetail(UINT buttonMsg, WORD wParamHi)
    {
        switch (buttonMsg)
        {
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN: return 1;
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN: return 2;
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN: return 3;
        case WM_XBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
            if (wParamHi == XBUTTON1)
                return 4;
            else if (wParamHi == XBUTTON2)
                return 5;
            else
                return 0;
        default:
            return 0;
        }
    }

    inline uint32_t getButtonUpEventDetail(UINT buttonMsg, WORD wParamHi)
    {
        switch (buttonMsg)
        {
        case WM_LBUTTONUP: return 1;
        case WM_MBUTTONUP: return 2;
        case WM_RBUTTONUP: return 3;
        case WM_XBUTTONUP:
            if (wParamHi == XBUTTON1)
                return 4;
            else if (wParamHi == XBUTTON2)
                return 5;
            else
                return 0;
        default:
            return 0;
        }
    }

    /// Win32_Window implements Win32 specific window creation, event handling and vulkan Surface setup.
    class VSG_DECLSPEC Win32_Window : public vsg::Inherit<vsg::Window, Win32_Window>
    {
    public:
        Win32_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        Win32_Window() = delete;
        Win32_Window(const Win32_Window&) = delete;
        Win32_Window operator=(const Win32_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return VK_KHR_WIN32_SURFACE_EXTENSION_NAME; }

        bool valid() const override { return _window; }

        bool visible() const override;

        void releaseWindow() override;

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;

        operator HWND() const { return _window; }

        /// handle Win32 event messages, return true if handled.
        virtual bool handleWin32Messages(UINT msg, WPARAM wParam, LPARAM lParam);

    protected:
        virtual ~Win32_Window();

        void _initSurface() override;

        HWND _window;
        bool _windowMapped = false;

        vsg::ref_ptr<KeyboardMap> _keyboard;
    };

    /// Use GetLastError() and FormatMessageA(..) to get the error number and error message and store them in a vsg::Exception.
    extern VSG_DECLSPEC vsg::Exception getLastErrorAsException(const std::string_view& prefix = {});

} // namespace vsgWin32

EVSG_type_name(vsgWin32::Win32_Window);
