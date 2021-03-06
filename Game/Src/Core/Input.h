#pragma once
#include <Windows.h>
#include "Keyboard.h"
#include "Mouse.h"
//#include <SimpleMath.h>
#include "RimfrostMath.hpp"
#include "Mouse.hpp"
#include <Inc\GamePad.h>

//this code is taken from a game project i was part of and it is not written by me, but i might poke around a little to make it work for this project
//Gunnar Cerne
class Window;
class Input
{
	friend Window;
public:
	using Keys = DirectX::Keyboard::Keys;

	/*enum Keys
	{
		A = DirectX::Keyboard::Keys::A, B = DirectX::Keyboard::Keys::B, C = DirectX::Keyboard::Keys::C, D = DirectX::Keyboard::Keys::D, E = DirectX::Keyboard::Keys::E, F = DirectX::Keyboard::Keys::F,
		G = DirectX::Keyboard::Keys::G, H = DirectX::Keyboard::Keys::H, I = DirectX::Keyboard::Keys::I, J = DirectX::Keyboard::Keys::J, K = DirectX::Keyboard::Keys::K, L = DirectX::Keyboard::Keys::L,
		M = DirectX::Keyboard::Keys::M, N = DirectX::Keyboard::Keys::N, O = DirectX::Keyboard::Keys::O, P = DirectX::Keyboard::Keys::P, Q = DirectX::Keyboard::Keys::Q,
		R = DirectX::Keyboard::Keys::R, S = DirectX::Keyboard::Keys::S, T = DirectX::Keyboard::Keys::T, U = DirectX::Keyboard::Keys::U, V = DirectX::Keyboard::Keys::V, W = DirectX::Keyboard::Keys::W,
		X = DirectX::Keyboard::Keys::X, Y = DirectX::Keyboard::Keys::Y, Z = DirectX::Keyboard::Keys::Z, Esc = DirectX::Keyboard::Keys::Escape, Space = DirectX::Keyboard::Keys::Space,
		Shift = DirectX::Keyboard::Keys::LeftShift, F1 = DirectX::Keyboard::Keys::F1, F2 = DirectX::Keyboard::Keys::F2, F3 = DirectX::Keyboard::Keys::F3, F4 = DirectX::Keyboard::Keys::F4,
		F5 = DirectX::Keyboard::Keys::F5, F6 = DirectX::Keyboard::Keys::F6, F7 = DirectX::Keyboard::Keys::F7, F8 = DirectX::Keyboard::Keys::F8, F9 = DirectX::Keyboard::Keys::F9, F10 = DirectX::Keyboard::Keys::F10,
		F11 = DirectX::Keyboard::Keys::F11, F12 = DirectX::Keyboard::Keys::F12
	};*/

	/*enum MouseKeys
	{
		LeftButton = 1, RightButton, MiddleButton
	};*/

	/*enum class MouseState
	{
		Absolute_Confined = 0,
		Absolute_UnConfined,
		Relative,
	};*/

	void operator=(Input const&) = delete;
	~Input();
	static Input& Get();
	static void Init(HWND wndHandle, int width, int height);
	static void Destroy();
	static bool Valid();
	void SetNewWidthAndHight(int width, int height);
	
	//rfm::Vector2 mousePos();
	//void mouseMovement(float& m_pitch, float& m_yaw);
	bool keyBeingPressed(DirectX::Keyboard::Keys key);
	bool keyPressed(DirectX::Keyboard::Keys key);
	bool keyReleased(DirectX::Keyboard::Keys key);
	//bool mouseBeingPressed(MouseKeys key);
	//bool mousePressed(MouseKeys key);
	//bool mouseReleased(MouseKeys key);
	//void SetMouseState(int code = 0);
	void update(long double dt);
	//int getLatestCode();
	long double getTime();
	Mouse& GetMouse();
	DirectX::GamePad& GamePad();
	DirectX::GamePad::State GamePadState() const;
	DirectX::GamePad::State OldGamePadState() const;
	//void setModeAbsolute();

	/*void ShowMouseCursor(bool yn);
	void ConfineCursor(bool yn);*/

private:
	static Input* instance;
	/*float m_mouseY;
	float m_mouseX;*/
	int m_width;
	int m_height;
	//int m_latestCode;
	HWND m_hwnd;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	//std::unique_ptr<DirectX::Mouse> m_xtkmouse;
	std::unique_ptr<Mouse> m_myMouse;
	std::unique_ptr<DirectX::GamePad> m_gamePad;
	DirectX::GamePad::State m_gamePadState;
	DirectX::GamePad::State m_gamePadStatePrevFrame;
	DirectX::Keyboard::KeyboardStateTracker m_keys;
	DirectX::Mouse::ButtonStateTracker m_mouseButtons;
	//bool m_mouseMode;
	//DirectX::Mouse::State mouse;
	long double m_frameTime;
private:
	Input(HWND wndHandle, int width, int height);
};