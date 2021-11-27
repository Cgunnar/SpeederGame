#pragma once
#include <cstdint>
namespace Geometry
{

	struct Quad
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
}