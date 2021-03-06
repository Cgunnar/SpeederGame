#pragma once

#include <wrl.h>
#include <d3d11.h>
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include "RimfrostMath.hpp"
#include "utilityTypes.h"
#include "GraphicsUtilityTypes.h"
#include "Material.h"
#include "boundingVolumes.h"

class LowLvlGfx;
class DX11;



namespace standardDescriptors
{
	extern const D3D11_SAMPLER_DESC g_sample_linear_wrap;
	extern const D3D11_SAMPLER_DESC g_sample_linear_clamp;
	extern const D3D11_SAMPLER_DESC g_sample_anisotropic_wrap;
	extern const D3D11_SAMPLER_DESC g_sample_anisotropic_clamp;
	extern const D3D11_SAMPLER_DESC g_sample_point_wrap;
	extern const D3D11_SAMPLER_DESC g_sample_point_clamp;
}

struct alignas(16) PointLight
{
	rfm::Vector3 position{ 0, 0, 0 };
	float lightStrength = 1;
	rfm::Vector3 color = { 1, 1, 1 };
	float constantAttenuation = 1;
	float LinearAttenuation = 0.1f;
	float exponentialAttenuation = 0.1f;
};

struct alignas(16) DirectionalLight
{
	rfm::Vector3 dir{ 0, -1, 0 };
	float lightStrength = 1;
	rfm::Vector3 color{ 1, 1, 1 };
};

class Shader
{
	friend LowLvlGfx;
public:
	Shader() = default;

private:
	Shader(uint32_t id) : m_id(id) {}
	uint32_t m_id = -1;
};

class ConstantBuffer
{
	friend LowLvlGfx;
public:
	ConstantBuffer() = default;
private:
	ConstantBuffer(uint32_t size) : m_size(size) {}
	uint32_t m_size = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

};
class Mesh;
class VertexBuffer
{
public:
	VertexBuffer() = default;
	uint32_t GetVertexCount() const { return vertexCount; }
private:
	friend LowLvlGfx;
	friend Mesh;
	uint32_t vertexStride = 0;
	uint32_t vertexCount = 0;
	uint32_t baseVertexLocation = 0;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
};

class IndexBuffer
{
public:
	IndexBuffer() = default;
	uint32_t GetIndexCount() const { return indexCount; }
private:
	friend LowLvlGfx;
	friend Mesh;
	uint32_t indexCount = 0;
	uint32_t startIndexLocation = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
};

class Texture2D
{
	friend LowLvlGfx;
	friend DX11;
public:
	Texture2D() = default;
	~Texture2D() = default;
	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
private:
	bool fixedRes = true;
};


class Sampler
{
	friend LowLvlGfx;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleState;
};

struct BlendState
{
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
};

struct Rasterizer
{
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;
};

class AssetManager;
class Mesh
{
	friend LowLvlGfx;
	friend AssetManager;
public:
	Mesh() = default;
	Mesh(const std::vector<Vertex_POS_NOR_UV>& vertices, const std::vector<uint32_t>& indices, AABB aabb = { {0,0,0},{0,0,0} });
	Mesh(const std::vector<Vertex_POS_NOR_UV>& vertices, const std::vector<uint32_t>& indices, uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, AABB aabb = { {0,0,0},{0,0,0} });
	Mesh(VertexBuffer vertices, IndexBuffer indices, uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, AABB aabb);
	Mesh(VertexBuffer vertices, IndexBuffer indices, AABB aabb);
	Mesh(const std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices, AABB aabb = { {0,0,0},{0,0,0} });
	Mesh(const std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices, uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, AABB aabb = { {0,0,0},{0,0,0} });
	uint32_t GetbaseVertexLocation() const { return baseVertexLocation; }
	uint32_t GetIndexCount() const { return indexCount; }
	uint32_t GetStartIndexLocation() const { return startIndexLocation; }
	AABB GetAABB() const { return m_aabb; }
	VertexBuffer vb;
	IndexBuffer ib;
	GID GetGID() const { return guid; }
private:
	AABB m_aabb;
	uint32_t startIndexLocation = 0;
	uint32_t baseVertexLocation = 0;
	uint32_t indexCount = 0;
	GID guid = GID::GenerateNew();
};

typedef size_t RenderUnitID;

struct RenderUnit
{
	Mesh GetMesh() const;
	GID meshID;
	Material material;
};

struct SubModel
{
	AABB aabb;
	std::vector<SubModel> subModels;
	std::vector<RenderUnitID> renderUnitIDs;
	RenderUnitID RenderUnitBegin, RenderUnitEnd;	// begin is renderUnitIDs[0], end is the last RenderUnitID+1 in the the tree of submodels
													// this is usefull if we want to give a function or class the ability to
													// loop over all Id:s but dont want to save the tree of submodels
};

struct Model : public SubModel
{
	
	VertexBuffer vb;
	IndexBuffer ib;
};

struct RendUnitIDAndTransform
{
	RendUnitIDAndTransform(RenderUnitID unitID, rfm::Transform worldMatrix, MaterialVariantEnum matType) : id(unitID),
		worldMatrix(worldMatrix), type(matType) {}
	RenderUnitID id;
	MaterialVariantEnum type;
	rfm::Transform worldMatrix;
};




