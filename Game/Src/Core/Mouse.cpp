#include "pch.hpp"
#include "Mouse.hpp"
#include <assert.h>
#include "FrameTimer.hpp"

Mouse::Mouse(std::function<Resolution()> getSizeCallback, std::function<HWND()> getHWNDCallback)
{
	m_getWindowSize = getSizeCallback;
	m_getHWND = getHWNDCallback;
}

void Mouse::update()
{
	auto [w, h] = m_getWindowSize();
	/*if (m_mouseState.LMBClicked || m_mouseState.LMBHeld || m_mouseState.LMBReleased || m_mouseState.RMBClicked || m_mouseState.RMBHeld || m_mouseState.RMBReleased)
	{
		m_mouseState.windowWidth = w;
		m_mouseState.windowHeight = h;
	}
	if (m_mouseState.deltaX != 0 || m_mouseState.deltaY != 0 || m_mouseState.deltaZ != 0)
	{
		m_mouseState.windowWidth = w;
		m_mouseState.windowHeight = h;
	}*/
	m_mouseState.windowWidth = w;
	m_mouseState.windowHeight = h;

	m_mouseState.deltaX = 0;
	m_mouseState.deltaY = 0;

	m_mouseState.LMBReleased = false;
	m_mouseState.LMBClicked = false;

	m_mouseState.RMBReleased = false;
	m_mouseState.RMBClicked = false;
}

MouseState Mouse::GetMouseState() const
{
	return this->m_mouseState;
}
void Mouse::showCursor(bool yn)
{
	m_showCursor = yn;
	ShowCursor(yn);
}
void Mouse::confineCursor(bool yn)
{
	m_cursorIsConfined = yn;
	if (m_cursorIsConfined && !m_windowOutOfFocus)
	{
		RECT r;
		GetClientRect(m_getHWND(), &r);
		MapWindowPoints(m_getHWND(), nullptr, (POINT*)&r, 2);
		ClipCursor(&r);
	}
	else
	{
		ClipCursor(nullptr);
	}
}
