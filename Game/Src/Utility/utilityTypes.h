#pragma once
#include <cstdint>
#include <random>

struct Resolution
{
	uint32_t width = 0;
	uint32_t height = 0;
};

struct GID
{
	GID(uint64_t id) : id(id){}
	GID() = default;

	operator const uint64_t& () const { return id; }

	static GID GenerateNew()
	{
		std::random_device rdev;
		std::mt19937 gen(rdev());
		std::uniform_int_distribution<int64_t> udis(1, INT64_MAX);
		return udis(gen);
	}
private:
	uint64_t id = 0;
};