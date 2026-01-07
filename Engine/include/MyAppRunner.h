#pragma once
#include <windows.h>
#include "MyGameEngine.h"

class MyGameEngine;

class MyAppRunner
{
private:
    static HWND m_hWnd;
    static bool m_fullscreenMode;
    static const UINT m_windowStyle = WS_OVERLAPPEDWINDOW;
    static RECT m_windowRect;

public:
    static int Run(MyGameEngine* pMyEngine, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hWnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};
