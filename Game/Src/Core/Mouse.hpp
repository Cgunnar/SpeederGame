#pragma once
#include <functional>
#include "utilityTypes.h"
struct MouseState
{
	int x = 0;
	int y = 0;
	int z = 0;


	float deltaX = 0;
	float deltaY = 0;
	float deltaZ = 0;

	bool LMBClicked = false;
	bool LMBReleased = false;
	bool LMBHeld = false;

	bool RMBClicked = false;
	bool RMBReleased = false;
	bool RMBHeld = false;

	uint32_t windowWidth = 0, windowHeight = 0;
};
class Window;
class Mouse
{
	friend Window;
public:
	Mouse(std::function<Resolution()> getResCallback,
		std::function<HWND()> getHWNDCallback);
	Mouse(const Mouse&) = delete;
	Mouse& operator=(const Mouse&) = delete;
	~Mouse() = default;

	void update();

	MouseState GetMouseState() const;

	void showCursor(bool yn);
	void confineCursor(bool yn);

private:


	MouseState m_mouseState{ 0 };

	bool m_showCursor = false;
	bool m_windowOutOfFocus = false;

	bool m_cursorIsConfined = true;


	std::function<Resolution()> m_getWindowSize;
	std::function<HWND()> m_getHWND;
};
