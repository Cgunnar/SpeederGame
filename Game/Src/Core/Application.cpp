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

using namespace rfe;
using namespace rfm;


Application::Application()
{
	m_window = new Window();
	LowLvlGfx::Init(m_window->GetHwnd(), m_window->GetClientSize());
	AssetManager::Init();
	m_renderer = new Renderer();
}

Application::~Application()
{
	delete m_renderer;
	AssetManager::Destroy();
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
	camera.getComponent<TransformComp>()->transform.setTranslation(0, 0, 1);
	camera.getComponent<TransformComp>()->transform.setRotationDeg(0, 0, 0);

	Geometry::Quad_POS_NOR_UV quad2;
	Entity quadEnt = EntityReg::createEntity();
	quadEnt.addComponent(TransformComp())->transform.setTranslation(0, 0, 5);
	quadEnt.addComponent(IndexedMeshComp())->indexBuffer = LowLvlGfx::CreateIndexBuffer(quad2.IndexData(), quad2.indexCount);
	quadEnt.getComponent<IndexedMeshComp>()->vertexBuffer = LowLvlGfx::CreateVertexBuffer(quad2.VertexData(), quad2.arraySize, quad2.vertexStride);
	

	
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

	quadEnt.addComponent(DiffuseTexturMaterialComp())->textureID = AssetManager::Get().AddTexture2D(myTexture);
	quadEnt.getComponent<DiffuseTexturMaterialComp>()->specularColor = { 1,0,0 };
	


	bool running = true;
	while (running)
	{
		FrameTimer::NewFrame();
		running = m_window->Win32MsgPump();
		if (!running)
		{
			break;
		}

		quadEnt.getComponent<TransformComp>()->transform.rotateDeg(0, 30 * FrameTimer::dt(), 0);

		
		LowLvlGfx::ClearRTV(0.1f, 0.2f, 0.4f, 0.0f, LowLvlGfx::GetBackBuffer());
		LowLvlGfx::ClearDSV(LowLvlGfx::GetDepthBuffer());

		LowLvlGfx::BindRTVs({ LowLvlGfx::GetBackBuffer() }, LowLvlGfx::GetDepthBuffer());

		m_renderer->Render(camera);

		LowLvlGfx::Present();
	}
}
