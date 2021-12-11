#include "pch.hpp"

#include "Window.h"
#include "LowLvlGfx.h"
#include "Input.h"
#include "utilityTypes.h"
#include <imgui.h>
#include <backends\imgui_impl_win32.h>

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



	//imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplWin32_Init(m_hWnd);



	ShowWindow(m_hWnd, SW_SHOWDEFAULT);

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = 0;
	rid.hwndTarget = nullptr;
	if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) assert(false); //failed to register device

	Input::Init(GetHwnd(), GetClientSize().width, GetClientSize().width);
	Input::getInput().m_myMouse = std::make_unique<Mouse>(
		[this]()->Resolution { return this->GetClientSize(); },
		[this]()->HWND { return this->GetHwnd(); });


	m_isStarting = false;
}

Window::~Window()
{
	Input::Destroy();

	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

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

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
	{
		return true;
	}

	switch (uMsg)
	{
	case WM_CLOSE:
	{
		m_isClosed = true;
		std::cout << "WM_CLOSE" << std::endl;
		//Input::getInput().SetMouseState(2);
		DestroyWindow(hwnd);
		return 0;
	}
	case WM_DESTROY:
	{
		std::cout << "WM_DESTROY" << std::endl;
		PostQuitMessage(0);
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
		if (Input::Valid())
		{
			Mouse& mouse = Input::getInput().GetMouse();
			mouse.confineCursor(mouse.m_cursorIsConfined);
		}
		return 0;
	}
	case WM_ACTIVATEAPP:
	{
		DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
		//DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);

		if (!Input::Valid()) return 0;
		Mouse& mouse = Input::getInput().GetMouse();
		if (wParam)
		{
			//this->GetGraphics().SetFullScreen(this->GetGraphics().ShouldBeFullScreen());
			mouse.m_windowOutOfFocus = false; //window is a friend of mouse to fix alt tab
			mouse.confineCursor(mouse.m_cursorIsConfined);
		}
		else
		{
			mouse.m_windowOutOfFocus = true;
		}
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		if (!Input::Valid()) return 0;
		Mouse& mouse = Input::getInput().GetMouse();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) break;
		
		POINTS p = MAKEPOINTS(lParam);
		mouse.m_mouseState.x = p.x;
		mouse.m_mouseState.y = p.y;
		break;
	}
	case WM_MOUSEWHEEL:
	{
		if (!Input::Valid()) return 0;
		Mouse& mouse = Input::getInput().GetMouse();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState.z += GET_WHEEL_DELTA_WPARAM(wParam);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		if (!Input::Valid()) return 0;
		Mouse& mouse = Input::getInput().GetMouse();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState.LMBClicked = true;
		mouse.m_mouseState.LMBHeld = true;
		//print("MousePos:  \n");
		break;
	}

	case WM_LBUTTONUP:
	{
		if (!Input::Valid()) return 0;
		Mouse& mouse = Input::getInput().GetMouse();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState.LMBReleased = true;
		mouse.m_mouseState.LMBHeld = false;
		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (!Input::Valid()) return 0;
		Mouse& mouse = Input::getInput().GetMouse();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState.RMBClicked = true;
		mouse.m_mouseState.RMBHeld = true;
		//print("MousePos:  \n");
		break;
	}

	case WM_RBUTTONUP:
	{
		if (!Input::Valid()) return 0;
		Mouse& mouse = Input::getInput().GetMouse();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState.RMBReleased = true;
		mouse.m_mouseState.RMBHeld = false;
		break;
	}

	case WM_INPUT:
	{
		if (!m_isStarting && !m_isClosed && Input::getInput().m_myMouse->m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		UINT bufferSize{};
		UINT errorCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &bufferSize, sizeof(RAWINPUTHEADER));
		assert(errorCode != -1);
		if (errorCode == -1) return 0;

		m_ridData.resize(bufferSize);
		errorCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, m_ridData.data(), &bufferSize, sizeof(RAWINPUTHEADER));
		assert(errorCode != -1);
		if (errorCode == -1) return 0;


		auto& myMouse = Input::getInput().m_myMouse;
		RAWINPUT& rawMouseInput = (RAWINPUT&)(*m_ridData.data());
		if (rawMouseInput.header.dwType == RIM_TYPEMOUSE)
		{
			if (rawMouseInput.data.mouse.lLastX || rawMouseInput.data.mouse.lLastY)
			{
				/*myMouse->m_mouseState.deltaX = static_cast<float>(rawMouseInput.data.mouse.lLastX);
				myMouse->m_mouseState.deltaY = static_cast<float>(rawMouseInput.data.mouse.lLastY);*/
				myMouse->m_mouseState.deltaX += static_cast<float>(rawMouseInput.data.mouse.lLastX);
				myMouse->m_mouseState.deltaY += static_cast<float>(rawMouseInput.data.mouse.lLastY);
			}
		}


		return 0;
	}

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_F11 && LowLvlGfx::IsValid())
		{
			if (LowLvlGfx::IsFullScreen())
				LowLvlGfx::LeaveFullScreen();
			else
				LowLvlGfx::EnterFullScreen();
		}
		/*if (wParam == VK_ESCAPE && Input::Valid())
		{
			Input::getInput().SetMouseState();
		}*/
		DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
		return 0;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
		return 0;
	}
	case WM_ACTIVATE:
	{
		if (!m_firstActivate)
		{
			m_firstActivate = true;
			break;
		}
		//if (wParam == WA_INACTIVE)
		//{
		//	// Changes to absolute WITHOUT changing internal 'code'.
		//	// This handles the case where the player alt+tabs without opening the menu
		//	// Internal state --> relative but we purposefully set the mode to absolute
		//	// So when we get back into the game, it returns back to relative.
		//	Input::getInput().setModeAbsolute();
		//	return 0;
		//}
		//if (wParam == WA_ACTIVE)
		//{
		//	// If alt+tabbed with relative --> get back in relative
		//	if (Input::getInput().getLatestCode() == 2)
		//		Input::getInput().SetMouseState(2);
		//	else // If alt+tabbed with absolute --> get back in absolute
		//		Input::getInput().SetMouseState(1);

		//	return 0;
		//}
		return 0;
	}
	/*case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
	{
		DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
		return 0;
	}*/

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}