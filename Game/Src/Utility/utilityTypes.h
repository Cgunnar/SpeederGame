#pragma once
#include <cstdint>
#include <random>

struct Resolution
{
	uint32_t width = 0;
	uint32_t height = 0;
};

enum class SimpleMesh
{
	Quad_POS_NOR_UV = 1,
	UVSphere_POS_NOR_UV_TAN_BITAN = 2,
	BOX_POS_NOR_UV = 3,
};

struct GID
{
	GID(uint64_t id) : id(id){}
	GID(SimpleMesh id) { this->id = static_cast<uint64_t>(id); };
	GID() = default;

	bool operator ==(int i) const { return id == i; }
	operator const uint64_t& () const { return id; }
	operator const bool () const { return id != 0; }

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