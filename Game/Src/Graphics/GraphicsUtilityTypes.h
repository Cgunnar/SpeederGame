#pragma once
#include "RimfrostMath.hpp"



enum class MeshFormat
{
	POS_NOR_UV = 0,
	POS_NOR_UV_TAN_BITAN,
};

enum class LoadTexFlag
{
	none = 0,
	GenerateMips = 1,
	LinearColorSpace = 2,
};

inline LoadTexFlag operator &(LoadTexFlag l, LoadTexFlag r)
{
	return (LoadTexFlag)((int)l & (int)r);
}
inline LoadTexFlag operator |(LoadTexFlag l, LoadTexFlag r)
{
	return (LoadTexFlag)((int)l | (int)r);
}
inline bool operator != (LoadTexFlag l, int r)
{
	return (bool)((int)l != r);
}
inline bool operator == (LoadTexFlag l, int r)
{
	return (bool)((int)l == r);
}

struct Vertex_POS_NOR_UV
{
	rfm::Vector3 position;
	rfm::Vector3 normal;
	rfm::Vector2 uv;
};

struct Triangle
{
	rfm::Vector3 position[3];
	rfm::Vector3 normal;
	rfm::Vector3& operator[](int index) { return position[index]; }
	operator bool() { return normal.length() > 0; }
};

struct Vertex_POS_UV
{
	rfm::Vector3 position;
	rfm::Vector2 uv;
};

struct Vertex_POS_NOR_UV_TAN_BITAN
{
	rfm::Vector3 position;
	rfm::Vector3 normal;
	rfm::Vector2 uv;
	rfm::Vector3 tangent;
	rfm::Vector3 biTangent;
};

struct alignas(16) VP
{
	rfm::Matrix V;
	rfm::Matrix P;
};

enum class RenderPassEnum
{
	none = 0,
};

enum class TextureTypes
{
	NONE = 0,
	DIFFUSE = 1,
	SPECULAR = 2,
	NORMAL = 4,


};


inline TextureTypes operator &(TextureTypes l, TextureTypes r)
{
	return (TextureTypes)((int)l & (int)r);
}
inline TextureTypes operator |(TextureTypes l, TextureTypes r)
{
	return (TextureTypes)((int)l | (int)r);
}


