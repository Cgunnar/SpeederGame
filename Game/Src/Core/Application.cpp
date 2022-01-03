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
#include "Curve_editor.h"


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

	EntityReg::StartScripts<CameraControllerScript, ShipContollerScript, TerrainScript>();

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

		auto ms = Input::Get().GetMouse().GetMouseState();

		m_scene->Update(static_cast<float>(FrameTimer::dt()));

		//ImGui::ShowDemoWindow();
		//static float v[5] = { 0.390f, 0.575f, 0.565f, 1.000f };
		//ImGui::Bezier( "easeOutSine", v );       // draw
		//float y = ImGui::BezierValue( 0.5f, v ); // x delta in [0..1] range
		
		m_renderer->RenderBegin(m_scene->GetCamera());
		m_renderer->RenderSkyBox(m_scene->sky);
		m_renderer->Render(m_scene->GetCamera(), m_scene->sunLight.GetComponent<DirectionalLightComp>()->dirLight);

		LowLvlGfx::EndFrame();
	}
}
