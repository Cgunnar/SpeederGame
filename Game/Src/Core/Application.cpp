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

using namespace rfe;
using namespace rfm;

struct alignas(16) VP
{
	Matrix V;
	Matrix P;
};


struct Quad
{
	Transform worldMatrix;
	Vector4 color;
	inline static VertexBuffer vertexBuffer;
	inline static IndexBuffer indexBuffer;
};
Application::Application()
{
	m_window = new Window();
	LowLvlGfx::Init(m_window->GetHwnd(), m_window->GetClientSize());
	m_renderer = new Renderer();
}

Application::~Application()
{
	delete m_renderer;
	LowLvlGfx::Destroy();
	delete m_window;
}

void SetSubResDataMips(const void* dataPtr, D3D11_SUBRESOURCE_DATA*& subResMipArray, int mipNumber, int stride)
{
	assert(dataPtr);

	subResMipArray = new D3D11_SUBRESOURCE_DATA[mipNumber];
	int SysMemPitch = stride;

	for (int i = 0; i < mipNumber; i++)
	{
		subResMipArray[i].pSysMem = dataPtr;
		subResMipArray[i].SysMemPitch = SysMemPitch;
		subResMipArray[i].SysMemSlicePitch = 0;
		SysMemPitch >>= 1;
	}
}

void Application::Run()
{
	Entity camera = EntityReg::createEntity();
	camera.addComponent(TransformComp());

	Entity quadEnt = EntityReg::createEntity();
	quadEnt.addComponent(TransformComp());
	quadEnt.addComponent(IndexedMeshComp());

	camera.getComponent<TransformComp>()->transform.setTranslation(0, 5, -7);
	camera.getComponent<TransformComp>()->transform.setRotationDeg(15, 0, 0);

	LowLvlGfx::SetViewPort(m_window->GetClientSize());

	Shader vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VertexShader.hlsl", ShaderType::VERTEXSHADER);
	//Shader pixelShader = LowLvlGfx::CreateShader("Src/Shaders/PixelShader.hlsl", ShaderType::PIXELSHADER);
	Shader pixelShader = LowLvlGfx::CreateShader("Src/Shaders/PS_FlatTexture.hlsl", ShaderType::PIXELSHADER);

	Geometry::Quad_POS_NOR_UV quad2;

	Quad::vertexBuffer = LowLvlGfx::CreateVertexBuffer(quad2.VertexData(), quad2.arraySize, quad2.vertexStride);
	Quad::indexBuffer = LowLvlGfx::CreateIndexBuffer(quad2.IndexData(), quad2.indexCount);

	ConstantBuffer worldMatrixCBuffer = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	ConstantBuffer vpCBuffer = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	ConstantBuffer colorCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Vector4), BufferDesc::USAGE::DYNAMIC });

	Vector4 lightPos(1,1,1);
	ConstantBuffer pointLightCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Vector4), BufferDesc::USAGE::DYNAMIC }, &lightPos);
	ConstantBuffer cameraCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Vector4), BufferDesc::USAGE::DYNAMIC });

	

	
	MyImageStruct im;
	readImage(im, "Assets/Hej.png");
	D3D11_TEXTURE2D_DESC desc;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Height = im.height;
	desc.Width = im.width;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MipLevels = im.mipNumber;

	D3D11_SUBRESOURCE_DATA* subResMipArray = nullptr;
	SetSubResDataMips(im.imagePtr, subResMipArray, im.mipNumber, im.stride);
	std::shared_ptr<Texture2D> myTexture = LowLvlGfx::CreateTexture2D(desc, subResMipArray);
	delete[] subResMipArray;


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	LowLvlGfx::CreateSRV(myTexture, &srvDesc);

	Sampler mySampler = LowLvlGfx::CreateSampler(standardSamplers::g_linear_wrap);


	

	VP vp;
	vp.P = Matrix(PIDIV4, 16.0f / 9.0f, 0.01f, 1000.0f);
	vp.V = inverse(*camera.getComponent<TransformComp>());

	Quad myBox;
	myBox.color = { 0.2f, 1.0f, 0.4f, 1.0f };
	myBox.worldMatrix.setTranslation(0, 3, 0);


	bool running = true;
	while (running)
	{
		FrameTimer::NewFrame();
		running = m_window->Win32MsgPump();
		if (!running)
		{
			break;
		}

		
		LowLvlGfx::ClearRTV(0.1f, 0.2f, 0.4f, 0.0f, LowLvlGfx::GetBackBuffer());
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
		/*LowLvlGfx::UpdateBuffer(colorCB, &myBox.color);
		LowLvlGfx::Bind(colorCB, ShaderType::PIXELSHADER, 0);*/
		LowLvlGfx::BindSRV(myTexture, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::Bind(mySampler, ShaderType::PIXELSHADER, 0);

		LowLvlGfx::DrawIndexed(Quad::indexBuffer.GetIndexCount(), 0, 0);
		LowLvlGfx::Present();
	}
}
