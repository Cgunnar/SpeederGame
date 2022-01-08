#include "pch.hpp"
#include "GraphicsResources.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"

namespace standardDescriptors
{
	const D3D11_SAMPLER_DESC g_sample_linear_wrap = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};
	const D3D11_SAMPLER_DESC g_sample_linear_clamp = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};

	const D3D11_SAMPLER_DESC g_sample_anisotropic_wrap = {
		.Filter = D3D11_FILTER_ANISOTROPIC,
		.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};
	
	const D3D11_SAMPLER_DESC g_sample_anisotropic_clamp = {
		.Filter = D3D11_FILTER_ANISOTROPIC,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};
	
	const D3D11_SAMPLER_DESC g_sample_point_wrap = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};

	const D3D11_SAMPLER_DESC g_sample_point_clamp = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};
}

Mesh::Mesh(const std::vector<Vertex_POS_NOR_UV>& vertices, const std::vector<uint32_t>& indices)
{
	ib = LowLvlGfx::CreateIndexBuffer(indices.data(), static_cast<uint32_t>(indices.size()));
	vb = LowLvlGfx::CreateVertexBuffer(reinterpret_cast<const float*>(vertices.data()), (uint32_t)vertices.size() * sizeof(Vertex_POS_NOR_UV), (uint32_t)sizeof(Vertex_POS_NOR_UV));
}

Mesh::Mesh(VertexBuffer vertices, IndexBuffer indices, uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation)
{
	this->ib = indices;
	this->vb = vertices;
	this->startIndexLocation = startIndexLocation;
	this->baseVertexLocation = baseVertexLocation;
	this->indexCount = indexCount;
}

Mesh::Mesh(VertexBuffer vertices, IndexBuffer indices)
{
	this->ib = indices;
	this->vb = vertices;
	this->startIndexLocation = ib.startIndexLocation;
	this->baseVertexLocation = vb.baseVertexLocation;
	this->indexCount = ib.indexCount;
}

Mesh::Mesh(const std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices)
{
	ib = LowLvlGfx::CreateIndexBuffer(indices.data(), static_cast<uint32_t>(indices.size()));
	vb = LowLvlGfx::CreateVertexBuffer(reinterpret_cast<const float*>(vertices.data()), (uint32_t)vertices.size() * sizeof(Vertex_POS_NOR_UV_TAN_BITAN), (uint32_t)sizeof(Vertex_POS_NOR_UV_TAN_BITAN));
	this->startIndexLocation = ib.startIndexLocation;
	this->baseVertexLocation = vb.baseVertexLocation;
	this->indexCount = ib.indexCount;
}

Mesh RenderUnit::GetMesh() const
{
	Mesh mesh = AssetManager::Get().GetMesh(meshID);
	return mesh;
}
