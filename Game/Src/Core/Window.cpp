#include "Window.h"

#include <iostream>
#include <assert.h>
#include <functional>

#include "LowLvlGfx.h"

Window* Window::s_windowInstance = nullptr;

Window::Window()
{
	assert(!s_windowInstance);
	s_windowInstance = this;
	m_hInst = GetModuleHandle(nullptr);

	WNDCLASSEX wc{};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = m_hInst;
	wc.lpszClassName = m_wndClassName;
	wc.lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {assert(s_windowInstance); return s_windowInstance->HandleMsg(hwnd, uMsg, wParam, lParam); };
	//wc.lpfnWndProc = HandleMsg;
	RegisterClassEx(&wc);

	RECT wr;
	wr.left = 100;
	wr.right = 1280 + wr.left;
	wr.top = 100;
	wr.bottom = 720 + wr.top;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	m_hWnd = CreateWindowEx(0, m_wndClassName, L"WindowText",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, m_hInst, nullptr); //last pointer is to some optional data of any kind that kan be usefull when getting messages

	ShowWindow(m_hWnd, SW_SHOWDEFAULT);


	//graphics


}

Window::~Window()
{
	UnregisterClass(m_wndClassName, m_hInst);
	s_windowInstance = nullptr;
}

bool Window::Win32MsgPump()
{
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		{
			return false;
		}
	}
	return true;
}

HWND Window::GetHwnd() const
{
	return m_hWnd;
}

Resolution Window::GetClientSize() const
{
	RECT winRect;
	if (GetClientRect(m_hWnd, &winRect))
	{
		return { static_cast<uint32_t>(winRect.right - winRect.left), static_cast<uint32_t>(winRect.bottom - winRect.top) };
	}
	else
	{
		assert(false);
		return Resolution();
	}
	
}

LRESULT Window::HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		std::cout << "WM_CLOSE" << std::endl;
		DestroyWindow(hwnd);
		return 0;
	}
	case WM_DESTROY:
	{
		std::cout << "WM_DESTROY" << std::endl;
		PostQuitMessage(0);
		return 0;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_F11 && LowLvlGfx::IsValid())
		{
			if (LowLvlGfx::IsFullScreen())
				LowLvlGfx::LeaveFullScreen();
			else
				LowLvlGfx::EnterFullScreen();
		}
		return 0;
	}
	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		std::cout << "WM_SIZE\twidth: " << width << " height: " << height << std::endl;
		auto res = GetClientSize();
		assert(width == res.width && height == res.height);

		LowLvlGfx::OnResize(res);

		return 0;
	}

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}