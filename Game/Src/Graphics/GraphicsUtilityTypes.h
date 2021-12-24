#pragma once
#include "RimfrostMath.hpp"

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
	phong,
	pbr,
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


enum class RenderFlag
{
	none = 0,
	wireframe = 1 << 1,
	noBackFaceCull = 1 << 2,
	alphaToCov = 1 << 3,
	alphaBlend = 1 << 4,
	end = 1 << 5,
};
inline RenderFlag operator &(RenderFlag l, RenderFlag r)
{
	return (RenderFlag)((int)l & (int)r);
}
inline RenderFlag operator |(RenderFlag l, RenderFlag r)
{
	return (RenderFlag)((int)l | (int)r);
}
inline bool operator != (RenderFlag l, int r)
{
	return (bool)((int)l != r);
}
inline bool operator == (RenderFlag l, int r)
{
	return (bool)((int)l == r);
}

inline void operator |= (RenderFlag& l, RenderFlag r)
{
	l = l | r;
}