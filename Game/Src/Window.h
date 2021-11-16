#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
class Window
{
public:
	Window();
	~Window();
	Window(const Window& other) = delete;
	Window& operator=(const Window& other) = delete;

	bool Win32MsgPump();
private:
	LRESULT CALLBACK HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static Window* s_windowInstance;
	const wchar_t* m_wndClassName{ L"wcName" };
	HINSTANCE m_hInst;
};

