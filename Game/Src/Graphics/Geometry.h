#pragma once
#include <cstdint>
#include <vector>
#include "RimfrostMath.hpp"
namespace Geometry
{

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
		struct VertexPOS_NOR_UV
		{
			rfm::Vector3 pos;
			rfm::Vector3 nor;
			rfm::Vector2 uv;
		};
		std::vector<VertexPOS_NOR_UV> vertices
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
}