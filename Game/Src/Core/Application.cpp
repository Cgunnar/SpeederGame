#include "Application.h"
#include "LowLvlGfx.h"
#include <iostream>
#include "Geometry.h"

#include "RimfrostMath.hpp"
#include "FrameTimer.hpp"
#include "ReadImg.hpp"


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
	inline static IndexBuffer indexBuffer;
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

	Geometry::Quad_POS_NOR_UV quad2;
	Quad::vertexBuffer = LowLvlGfx::CreateVertexBuffer(quad2.VertexData(), quad2.arraySize, quad2.vertexStride);
	Quad::indexBuffer = LowLvlGfx::CreateIndexBuffer(quad2.IndexData(), quad2.indexCount);

	ConstantBuffer worldMatrixCBuffer = LowLvlGfx::CreateConstantBuffer({ sizeof(rf::Matrix), BufferDesc::USAGE::DYNAMIC });
	ConstantBuffer vpCBuffer = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(rf::Matrix), BufferDesc::USAGE::DYNAMIC });
	ConstantBuffer colorCB = LowLvlGfx::CreateConstantBuffer({ sizeof(rf::Vector4), BufferDesc::USAGE::DYNAMIC });

	/*
	MyImageStruct im;
	readImage(&im, )
	D3D11_TEXTURE2D_DESC desc;*/
	


	rf::Transform camera;
	camera.setTranslation(0, 3, -10);
	camera.setRotationDeg(15, 0, 0);

	VP vp;
	vp.P = rf::Matrix(rf::PIDIV4, 16.0f / 9.0f, 0.01f, 1000.0f);
	vp.V = rf::inverse(camera);

	Quad myBox;
	myBox.color = { 0.2f, 1.0f, 0.4f, 1.0f };
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
		LowLvlGfx::Bind(Quad::indexBuffer);
		LowLvlGfx::Bind(vertexShader);
		LowLvlGfx::Bind(pixelShader);
		LowLvlGfx::UpdateBuffer(vpCBuffer, &vp);
		LowLvlGfx::Bind(vpCBuffer, ShaderType::VERTEXSHADER, 1);
		LowLvlGfx::UpdateBuffer(worldMatrixCBuffer, &myBox.worldMatrix);
		LowLvlGfx::Bind(worldMatrixCBuffer, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::UpdateBuffer(colorCB, &myBox.color);
		LowLvlGfx::Bind(colorCB, ShaderType::PIXELSHADER, 0);

		LowLvlGfx::DrawIndexed(Quad::indexBuffer.GetIndexCount(), 0, 0);
		LowLvlGfx::Present();
	}
}
