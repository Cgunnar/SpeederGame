#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Transform.hpp"

namespace rfm
{
	constexpr float PI = 3.141592654f;
	constexpr float PI2 = 6.283185307f;
	constexpr float PIDIV2 = 1.570796327f;
	constexpr float PIDIV4 = 0.785398163f;

	constexpr inline float InvLerp(float a, float b, float t)
	{
		return a != b ? (t - a) / (b - a) : a;
	}

	constexpr inline float Remap(float x, float in_min, float in_max, float out_min, float out_max)
	{
		return std::lerp(out_min, out_max, InvLerp(in_min, in_max, x));
	}
}
