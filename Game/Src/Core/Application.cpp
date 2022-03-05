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
	FrameTimer::Init();
	WorkerThreads::Init();
	m_window = new Window();
	LowLvlGfx::Init(m_window->GetHwnd(), m_window->GetClientSize(), { 1280, 720});
	
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
		double dt = FrameTimer::NewFrame();
		running = m_window->Win32MsgPump();
		if (!running)
		{
			break;
		}
		LowLvlGfx::BeginFrame();
		Input::Get().update(dt);

		if (Input::Get().keyPressed(Input::Keys::Escape))
		{
			Mouse& mouse = Input::Get().GetMouse();
			mouse.SetMode(~mouse.GetMode());
			m_scene->GetCamera().GetComponent<CameraControllerScript>()->ToggleCameraLock();
		}

		if (Input::Get().keyPressed(Input::Keys::F11) && LowLvlGfx::IsValid())
		{
			if (LowLvlGfx::IsFullScreen())
				LowLvlGfx::LeaveFullScreen();
			else
				LowLvlGfx::EnterFullScreen();
		}

		ImGui::Begin("Settings");
		if (ImGui::BeginMenu("Graphics"))
		{
			if (ImGui::MenuItem("native res"))
			{
				LowLvlGfx::SetRenderResolution(LowLvlGfx::GetNativeResolution());
				std::cout << "native res" << std::endl;
			}
			if (ImGui::MenuItem("720p"))
			{
				LowLvlGfx::SetRenderResolution({ 1280, 720 });
				std::cout << "720p" << std::endl;
			}

			ImGui::EndMenu();
		}
		ImGui::End();

		m_physicsEngine.Run(dt);
		m_scene->Update(static_cast<float>(dt));
		
		MemoryInfo memInfo = LowLvlGfx::GetMemoryUsage();
		ImGui::Text(memInfo.adapterName.c_str());
		ImGui::Text("vram usage: %u MB", memInfo.applicationMemoryUsage / 1000000);

		m_renderer->RenderToEnvMap(m_scene->GetCamera().GetComponent<TransformComp>()->transform.getTranslation(), *m_scene);
		m_renderer->RenderScene(*m_scene);
		m_renderer->RenderPostProcess();

		LowLvlGfx::EndFrame();

	}
}
