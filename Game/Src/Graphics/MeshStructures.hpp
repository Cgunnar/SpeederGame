#pragma once
#include <string>
#include <vector>
#include "GraphicsUtilityTypes.h"

//#include "Material.hpp"





struct EngineMeshSubset
{
	std::string name;
	TextureTypes texTypes;
	//Rimfrost::Material material;

	unsigned int vertexCount;
	unsigned int vertexStart;

	unsigned int indexStart;
	unsigned int indexCount;

	std::string filePath;
	std::string diffuseFileName;
	float color[3];		// used if diffuseFilePath == ""

	std::string specularFileName;
	std::string normalFileName;
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