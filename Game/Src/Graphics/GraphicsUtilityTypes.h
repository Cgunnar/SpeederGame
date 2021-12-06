#pragma once
#include "RimfrostMath.hpp"

typedef size_t MeshID;

struct Vertex_POS_NOR_UV
{
	rfm::Vector3 position;
	rfm::Vector3 normal;
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