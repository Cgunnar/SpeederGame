#pragma once
#include <random>

inline float GenRandFloat(float min, float max)
{
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_real_distribution<> distr(min, max);
	return static_cast<float>(distr(eng));
}
inline float GenRandFloat(float min, float max, unsigned int seed)
{
	std::mt19937 eng(seed);
	std::uniform_real_distribution<> distr(min, max);
	return static_cast<float>(distr(eng));
}
inline int GenRand(int min, int max)
{
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(min, max);
	return distr(eng);
}
inline int GenRand(int min, int max, unsigned int seed)
{
	std::mt19937 eng(seed);
	std::uniform_int_distribution<> distr(min, max);
	return distr(eng);
}
