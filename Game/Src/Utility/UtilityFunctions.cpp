#include "pch.hpp"
#include "UtilityFunctions.h"

namespace util
{

	std::vector<uint8_t> FloatToCharRGB(const float* src, int width, int height)
	{
		std::vector<uint8_t> charVec;
		charVec.resize(static_cast<size_t>(width) * height * 3);
		for (int i = 0; i < width * height * 3; i++)
		{
			charVec[i] = ToUint8(src[i]);
		}
		return charVec;
	}
}
