#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include "utilityTypes.h"
class Window
{
public:
	Window();
	~Window();
	Window(const Window& other) = delete;
	Window& operator=(const Window& other) = delete;

	bool Win32MsgPump();
	HWND GetHwnd() const;
	Resolution GetClientSize() const;
private:
	LRESULT CALLBACK HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool m_firstActivate = false;
	static Window* s_windowInstance;
	const wchar_t* m_wndClassName{ L"wcName" };
	HINSTANCE m_hInst;
	HWND m_hWnd;
	bool m_isClosed = false;
	bool m_isStarting = true;
	bool m_wasFullscreenWhenOnOutOfFocus = false;

	std::vector<unsigned char> m_ridData;
};

