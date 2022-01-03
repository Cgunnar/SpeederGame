#pragma once

namespace util
{
	std::vector<uint8_t> FloatToCharRGB(const float* src, int width, int height);



	inline constexpr uint8_t ToUint8(float x) noexcept
	{
		return (x <= 0.0) ? 0 : (1.0 <= x) ? 255 : static_cast<uint8_t>(x * 255.0 + 0.5);
	}
}