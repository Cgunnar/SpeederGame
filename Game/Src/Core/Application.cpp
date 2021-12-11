#include "pch.hpp"

#include "Application.h"
#include "LowLvlGfx.h"
#include "Geometry.h"

#include "RimfrostMath.hpp"
#include "FrameTimer.hpp"
#include "ReadImg.hpp"
#include "rfEntity.hpp"
#include "StandardComponents.h"
#include "RenderComponents.h"
#include "AssetManager.h"
#include "Input.h"


using namespace rfe;
using namespace rfm;


Application::Application()
{
	m_window = new Window();
	LowLvlGfx::Init(m_window->GetHwnd(), m_window->GetClientSize());
	
	AssetManager::Init();
	m_renderer = new Renderer();
	m_scene = new Scene();
}

Application::~Application()
{
	delete m_scene;
	delete m_renderer;
	AssetManager::Destroy();
	
	LowLvlGfx::Destroy();
	delete m_window;
}



void Application::Run()
{
	//Input::getInput().GetMouse().confineCursor(true);

	bool running = true;
	while (running)
	{
		FrameTimer::NewFrame();
		running = m_window->Win32MsgPump();
		if (!running)
		{
			break;
		}
		LowLvlGfx::BeginFrame();
		Input::getInput().update(FrameTimer::dt());

		auto ms = Input::getInput().GetMouse().GetMouseState();

		m_scene->Update(FrameTimer::dt());

		LowLvlGfx::ClearRTV(0.1f, 0.2f, 0.4f, 0.0f, LowLvlGfx::GetBackBuffer());
		LowLvlGfx::ClearDSV(LowLvlGfx::GetDepthBuffer());
		LowLvlGfx::BindRTVs({ LowLvlGfx::GetBackBuffer() }, LowLvlGfx::GetDepthBuffer());
		m_renderer->Render(m_scene->GetCamera());

		LowLvlGfx::EndFrame();
	}
}
