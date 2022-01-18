
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>



class Vector2
{
public:
	Vector2(float x, float y) : x(x), y(y) {}
	Vector2(float val = 0) : x(val), y(val) {}
	float x = 0;
	float y = 0;
};

float GenRandFloat(float min, float max, unsigned int seed)
{
	//std::ranlux48_base eng(seed);
	//std::minstd_rand eng(seed);
	//std::mt19937 eng(seed);
	std::ranlux48 eng(seed);
	std::uniform_real_distribution<> distr(min, max);
	auto t = static_cast<float>(distr(eng));
	return t;
}

int main()
{
	unsigned int seed = 42;
	std::vector<Vector2> randVec;
	int maxIt = 25;
	for (int i = 0; i < maxIt; i++)
	{
		Vector2 v;
		v.x = GenRandFloat(0, 240, seed + i); 
		v.y = GenRandFloat(0, 240, seed + i + v.x);
		randVec.push_back(v);
	}
	std::sort(randVec.begin(), randVec.end(), [](auto a, auto b) {return a.x < b.x; });
	for (auto& f : randVec)
	{
		std::cout << f.x << " " << f.y << std::endl;
	}
	return 0;
}