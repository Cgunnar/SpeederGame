#include "Application.h"
#include "LowLvlGfx.h"
#include <iostream>
#include "Geometry.h"

#include "RimfrostMath.hpp"
#include "FrameTimer.hpp"


struct alignas(16) VP
{
	rf::Matrix V;
	rf::Matrix P;
};


struct Quad
{
	rf::Transform worldMatrix;
	rf::Vector4 color;
	inline static VertexBuffer vertexBuffer;
};
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
	LowLvlGfx::SetViewPort(m_window->GetClientSize());

	Shader vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VertexShader.hlsl", ShaderType::VERTEXSHADER);
	Shader pixelShader = LowLvlGfx::CreateShader("Src/Shaders/PixelShader.hlsl", ShaderType::PIXELSHADER);


	Geometry::Quad quad = Geometry::Quad();
	Quad::vertexBuffer = LowLvlGfx::CreateVertexBuffer(quad.data, quad.arraySize, quad.vertexStride, quad.vertexOffset);

	ConstantBuffer worldMatrixCBuffer = LowLvlGfx::CreateConstantBuffer({ sizeof(rf::Matrix), BufferDesc::USAGE::DYNAMIC });
	ConstantBuffer vpCBuffer = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(rf::Matrix), BufferDesc::USAGE::DYNAMIC });
	ConstantBuffer colorCB = LowLvlGfx::CreateConstantBuffer({ sizeof(rf::Vector4), BufferDesc::USAGE::DYNAMIC });


	rf::Transform camera;
	camera.setTranslation(0, 3, -10);
	camera.setRotationDeg(15, 0, 0);

	VP vp;
	vp.P = rf::Matrix(rf::PIDIV4, 16.0f / 9.0f, 0.01f, 1000.0f);
	vp.V = rf::inverse(camera);



	Quad myFloor;
	myFloor.color = { 1, 0.2, 0.2, 1 };
	myFloor.worldMatrix.setRotationDeg(90, 0, 0);
	myFloor.worldMatrix.setScale(5);

	Quad myBox;
	myBox.color = { 0.2, 1, 0.4, 1 };
	myBox.worldMatrix.setTranslation(0, 4, 0);


	bool running = true;
	while (running)
	{
		FrameTimer::NewFrame();
		running = m_window->Win32MsgPump();
		if (!running)
		{
			break;
		}
		LowLvlGfx::ClearRTV(0.2f, 0.2f, 0.2f, 0.0f, LowLvlGfx::GetBackBuffer());
		LowLvlGfx::ClearDSV(LowLvlGfx::GetDepthBuffer());
		LowLvlGfx::BindRTVs({ LowLvlGfx::GetBackBuffer() }, LowLvlGfx::GetDepthBuffer());
		LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		LowLvlGfx::Bind(Quad::vertexBuffer);
		LowLvlGfx::Bind(vertexShader);
		LowLvlGfx::Bind(pixelShader);
		LowLvlGfx::UpdateBuffer(vpCBuffer, &vp);
		LowLvlGfx::Bind(vpCBuffer, ShaderType::VERTEXSHADER, 1);

		LowLvlGfx::UpdateBuffer(worldMatrixCBuffer, &myFloor.worldMatrix);
		LowLvlGfx::Bind(worldMatrixCBuffer, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::UpdateBuffer(colorCB, &myFloor.color);
		LowLvlGfx::Bind(colorCB, ShaderType::PIXELSHADER, 0);

		LowLvlGfx::Draw(Quad::vertexBuffer.GetVertexCount());

		LowLvlGfx::UpdateBuffer(worldMatrixCBuffer, &myBox.worldMatrix);
		LowLvlGfx::UpdateBuffer(colorCB, &myBox.color);
		
		
		LowLvlGfx::Draw(Quad::vertexBuffer.GetVertexCount());
		LowLvlGfx::Present();
	}
}
