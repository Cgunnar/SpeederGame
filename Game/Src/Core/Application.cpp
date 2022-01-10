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
#include "NativeScriptCollection.h"
#include "imgui.h"
#include "WorkerThreads.h"


using namespace rfe;
using namespace rfm;


Application::Application()
{
	WorkerThreads::Init();
	m_window = new Window();
	LowLvlGfx::Init(m_window->GetHwnd(), m_window->GetClientSize());
	
	AssetManager::Init();
	m_renderer = new Renderer();
	m_scene = new Scene();
}

Application::~Application()
{
	delete m_scene;
	WorkerThreads::Destroy();
	delete m_renderer;
	AssetManager::Destroy();
	
	LowLvlGfx::Destroy();
	delete m_window;
}


void Application::Run()
{
	
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
		Input::Get().update(FrameTimer::dt());

		if (Input::Get().keyPressed(Input::Esc))
		{
			Mouse& mouse = Input::Get().GetMouse();
			mouse.SetMode(~mouse.GetMode());
			m_scene->GetCamera().GetComponent<CameraControllerScript>()->ToggleCameraLock();
		}

		if (Input::Get().keyPressed(Input::F11) && LowLvlGfx::IsValid())
		{
			if (LowLvlGfx::IsFullScreen())
				LowLvlGfx::LeaveFullScreen();
			else
				LowLvlGfx::EnterFullScreen();
		}

		m_scene->Update(static_cast<float>(FrameTimer::dt()));
		
		MemoryInfo memInfo = LowLvlGfx::GetMemoryUsage();
		ImGui::Text(memInfo.adapterName.c_str());
		ImGui::Text("vram usage: %u MB", memInfo.applicationMemoryUsage / 1000000);

		m_renderer->RenderBegin(m_scene->GetCamera());
		m_renderer->RenderSkyBox(m_scene->sky);
		m_renderer->Render(m_scene->GetCamera(), m_scene->sunLight.GetComponent<DirectionalLightComp>()->dirLight);

		LowLvlGfx::EndFrame();
	}
}
