#pragma once
#include <string>
#include <vector>
#include "GraphicsUtilityTypes.h"

//#include "Material.hpp"

enum class MaterialProperties
{
	NONE = 0,
	DIFFUSE_MAP = 1 << 0,
	SPECULAR_MAP = 1 << 1,
	NORMAL_MAP = 1 << 2,
	SHININESS = 1 << 3,
	DIFFUSE_COLOR = 1 << 4,
	SPECULAR_COLOR = 1 << 5,
	AMBIENT_COLOR = 1 << 6,

	ALPHA_BLENDING = 1 << 7,
	ALPHA_BLENDING_CONSTANS_OPACITY = 1 << 8,
	ALPHA_TESTING = 1 << 9,

	IS_EMISSIVE = 1 << 10,

	NO_BACKFACE_CULLING = 1 << 11,
	WIREFRAME = 1 << 12,

};

inline MaterialProperties operator &(MaterialProperties l, MaterialProperties r)
{
	return (MaterialProperties)((int)l & (int)r);
}
inline MaterialProperties operator |(MaterialProperties l, MaterialProperties r)
{
	return (MaterialProperties)((int)l | (int)r);
}

inline bool operator != (MaterialProperties l, int r)
{
	return (bool)((int)l != r);
}

struct EngineMaterial
{
	std::string name;
	MaterialProperties properties = MaterialProperties::NONE;
	rfm::Vector3 colorDiffuse = { 0,0,0 };
	rfm::Vector3 colorAmbient = { 0,0,0 };
	rfm::Vector3 colorSpecular = { 0,0,0 }; // what does this have to do with the material?, it should be a property of the light source
	rfm::Vector3 colorEmissive = { 0,0,0 }; //fix later
	rfm::Vector3 colorTransparent = { 0,0,0 };
	float opacity = 1; // why do i need this?
	float shininess = 0;

	std::string diffuseMapPath = "";
	std::string specularMapPath = "";
	std::string normalMapPath = "";
};


struct EngineMeshSubset
{
	std::string name;
	//TextureTypes texTypes;
	EngineMaterial material;

	unsigned int vertexCount;
	unsigned int vertexStart;

	unsigned int indexStart;
	unsigned int indexCount;

	//std::string filePath;
	//std::string diffuseFileName;
	//float color[3];		// used if diffuseFilePath == ""

	//std::string specularFileName;
	//std::string normalFileName;
};


struct SubMeshTree
{
	std::vector<SubMeshTree> nodes;
	std::vector<EngineMeshSubset> subMeshes;
	std::vector<size_t> subMeshesIndex;
};

class AssimpLoader;
struct EngineMeshData
{
	friend AssimpLoader;

private:
	std::vector<uint32_t> indices;
	std::vector<Vertex_POS_NOR_UV> vertices;
	std::vector<Vertex_POS_NOR_UV_TAN_BITAN> verticesTBN;
	bool hasNormalMap = false;

public:
	std::vector<EngineMeshSubset> subMeshesVector;
	SubMeshTree subsetsInfo;

	bool hasNormalMaps() const
	{
		return this->hasNormalMap;
	}

	float* getVertextBuffer() const
	{
		if (hasNormalMap)
			return (float*)this->verticesTBN.data();
		else
			return (float*)this->vertices.data();
	}
	size_t getVertexSize() const
	{
		if (hasNormalMap)
			return sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
		else
			return sizeof(Vertex_POS_NOR_UV);
	}
	size_t getVertexCount() const
	{
		if (hasNormalMap)
			return verticesTBN.size();
		else
			return vertices.size();
	}
	size_t getIndicesCount() const
	{
		return indices.size();
	}
	const unsigned int* getIndicesData() const
	{
		return indices.data();
	}
};