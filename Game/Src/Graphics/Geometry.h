#pragma once

#include "RimfrostMath.hpp"
#include "GraphicsUtilityTypes.h"
#include "boundingVolumes.h"
namespace Geometry
{
	void CalcTanAndBiTan(std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices);
	struct Quad_POS_UV
	{
		const float data[30] = {
			-1.0, -1.0, 0.0, 0.0, 1.0,
			-1.0, 1.0, 0.0, 0.0, 0.0,
			1.0, -1.0, 0.0, 1.0, 1.0,

			-1.0, 1.0, 0.0, 0.0, 0.0,
			1.0, 1.0, 0.0, 1.0, 0.0,
			1.0, -1.0, 0.0, 1.0, 1.0
		};
		uint32_t arraySize = 6 * 5 * 4;
		uint32_t vertexStride = 5 * 4;
		uint32_t vertexOffset = 0;
		//InputLayout::Layout layout = InputLayout::Layout::POS_TEX;
	};

	struct Quad_POS_NOR_UV
	{
		
		std::vector<Vertex_POS_NOR_UV> vertices
		{
			{ rfm::Vector3(-1, 1, 0.0),  rfm::Vector3(0.0, 0.0, -1.0), rfm::Vector2(0.0, 0.0) },
			{ rfm::Vector3(1, 1, 0.0),   rfm::Vector3(0.0, 0.0, -1.0), rfm::Vector2(1.0, 0.0) },
			{ rfm::Vector3(1, -1, 0.0),  rfm::Vector3(0.0, 0.0, -1.0), rfm::Vector2(1.0, 1.0) },
			{ rfm::Vector3(-1, -1, 0.0), rfm::Vector3(0.0, 0.0, -1.0), rfm::Vector2(0.0, 1.0) }
		};

		std::vector<uint32_t> indices = {
			0, 1, 2,
			0, 2, 3
		};

		uint32_t indexCount = 6;
		uint32_t arraySize = 8 * 4 * 4;
		uint32_t vertexStride = 8 * 4;

		float* VertexData() { return (float*)vertices.data(); }
		uint32_t* IndexData() { return (uint32_t*)indices.data(); }
	};

	class Sphere_POS_NOR_UV
	{
	public:
		Sphere_POS_NOR_UV(int tessellation = 16, float radius = 1);
		const uint32_t IndexCount() const { return static_cast<uint32_t>(indices.size()); }
		const uint32_t ArraySize() const { return static_cast<uint32_t>(vertices.size() * vertexStride); }
		float* VertexData() { return (float*)vertices.data(); }
		uint32_t* IndexData() { return (uint32_t*)indices.data(); }

		static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV);
	private:
		std::vector<Vertex_POS_NOR_UV> vertices;
		std::vector<uint32_t> indices;
		uint32_t indexCount = 0;
		uint32_t arraySize  = 0;		
	};

	class Sphere_POS_NOR_UV_TAN_BITAN
	{
	public:
		Sphere_POS_NOR_UV_TAN_BITAN(int tessellation = 16, float radius = 1);
		const uint32_t IndexCount() const { return static_cast<uint32_t>(indices.size()); }
		const uint32_t ArraySize() const { return static_cast<uint32_t>(vertices.size() * vertexStride); }
		float* VertexData() { return (float*)vertices.data(); }
		uint32_t* IndexData() { return (uint32_t*)indices.data(); }

		static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
	private:
		std::vector<Vertex_POS_NOR_UV_TAN_BITAN> vertices;
		std::vector<uint32_t> indices;
		uint32_t indexCount = 0;
		uint32_t arraySize = 0;
	};



	class AABB_POS_NOR_UV
	{
	public:
		AABB_POS_NOR_UV(AABB aabb);

		const uint32_t IndexCount() const { return static_cast<uint32_t>(indices.size()); }
		const uint32_t ArraySize() const { return static_cast<uint32_t>(vertices.size() * vertexStride); }
		float* VertexData() { return (float*)vertices.data(); }
		uint32_t* IndexData() { return (uint32_t*)indices.data(); }

		static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV);
	private:
		std::vector<Vertex_POS_NOR_UV> vertices;
		std::vector<uint32_t> indices;
		uint32_t indexCount = 0;
		uint32_t arraySize = 0;
	};
}