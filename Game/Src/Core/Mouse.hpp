#pragma once
#include <functional>
#include "utilityTypes.h"
struct MouseState
{
	int x = 0;
	int y = 0;
	int z = 0;

	float mouseCof = 0.15f;
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

	enum class Mode
	{
		Visible = 1,
		Confined = 2
	} m_mode = Mode::Visible;

	void SetMode(Mode mode);
	Mode GetMode() const { return m_mode; };

private:


	MouseState m_mouseState0{ 0 };
	MouseState m_mouseState1{ 0 };

	bool m_showCursor = false;
	bool m_windowOutOfFocus = false;

	bool m_cursorIsConfined = true;


	std::function<Resolution()> m_getWindowSize;
	std::function<HWND()> m_getHWND;
};


inline Mouse::Mode operator ~(Mouse::Mode m)
{
	return (Mouse::Mode)~(int)m;
}

inline Mouse::Mode operator &(Mouse::Mode l, Mouse::Mode r)
{
	return (Mouse::Mode)((int)l & (int)r);
}
inline Mouse::Mode operator |(Mouse::Mode l, Mouse::Mode r)
{
	return (Mouse::Mode)((int)l | (int)r);
}
