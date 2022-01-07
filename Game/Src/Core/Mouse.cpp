#include "pch.hpp"
#include "Mouse.hpp"
#include <assert.h>
#include "FrameTimer.hpp"

Mouse::Mouse(std::function<Resolution()> getSizeCallback, std::function<HWND()> getHWNDCallback)
{
	m_getWindowSize = getSizeCallback;
	m_getHWND = getHWNDCallback;
}

void Mouse::SetMode(Mode mode)
{
	m_mode = mode;
	confineCursor((mode & Mode::Confined) != (Mode)0);
	showCursor((mode & Mode::Visible) != (Mode)0);
}

void Mouse::update()
{
	auto [w, h] = m_getWindowSize();
	m_mouseState0.windowWidth = w;
	m_mouseState0.windowHeight = h;


	m_mouseState1 = m_mouseState0;
	

	m_mouseState0.deltaX = 0;
	m_mouseState0.deltaY = 0;
	m_mouseState0.deltaZ = 0;

	m_mouseState0.LMBReleased = false;
	m_mouseState0.LMBClicked = false;

	m_mouseState0.RMBReleased = false;
	m_mouseState0.RMBClicked = false;
}

MouseState Mouse::GetMouseState() const
{
	return this->m_mouseState1;
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
