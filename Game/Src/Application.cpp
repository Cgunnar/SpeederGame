#include "Application.h"
#include "LowLvlGfx.h"
#include <iostream>
#include "Geometry.h"

Application::Application()
{
	m_window = new Window();
	LowLvlGfx::Init(m_window->GetHwnd(), m_window->GetClientSize());
}

Application::~Application()
{
	LowLvlGfx::Destroy();
	delete m_window;
}

void Application::Run()
{

	Geometry::Quad quad = Geometry::Quad();
	VertexBuffer quadVB = LowLvlGfx::CreateVertexBuffer(quad.data, quad.arraySize, quad.vertexStride, quad.vertexOffset);

	bool running = true;
	while (running)
	{
		running = m_window->Win32MsgPump();
		if (!running)
		{
			break;
		}
		LowLvlGfx::ClearRTV(0.2f, 0.2f, 0.2f, 0.0f, LowLvlGfx::GetBackBuffer());
		LowLvlGfx::ClearDSV(LowLvlGfx::GetDepthBuffer());




		LowLvlGfx::Bind(quadVB);
		//LowLvlGfx::Draw(quadVB.GetVertexCount());
		LowLvlGfx::Present();
	}
}
