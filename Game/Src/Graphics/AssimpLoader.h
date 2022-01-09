#pragma once
#pragma warning(push, 0)
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#pragma warning(pop)
#include <string>
#include <vector>
#include "GraphicsUtilityTypes.h"
#include "Material.h"
#include "boundingVolumes.h"



//struct EngineMaterial
//{
//	std::string name;
//	MaterialProperties properties = MaterialProperties::NONE;
//	rfm::Vector3 colorDiffuse = { 0,0,0 };
//	rfm::Vector3 colorAmbient = { 0,0,0 };
//	rfm::Vector3 colorSpecular = { 0,0,0 }; // what does this have to do with the material?, it should be a property of the light source
//	rfm::Vector3 colorEmissive = { 0,0,0 }; //fix later
//	rfm::Vector3 colorTransparent = { 0,0,0 };
//	float opacity = 1; // why do i need this?
//	float shininess = 0;
//
//	std::string diffuseMapPath = "";
//	std::string specularMapPath = "";
//	std::string normalMapPath = "";
//	std::string metallicroughnessPath = "";
//
//};


struct EngineMeshSubset
{
	std::string name;
	Material pbrMaterial;
	AABB aabb;
	unsigned int vertexCount;
	unsigned int vertexStart;
	unsigned int indexStart;
	unsigned int indexCount;
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
	float* getVertextBuffer(MeshFormat format) const
	{
		switch (format)
		{
		case MeshFormat::POS_NOR_UV:
			return (float*)this->vertices.data();
		case MeshFormat::POS_NOR_UV_TAN_BITAN:
			return (float*)this->verticesTBN.data();
		}
		return nullptr;
	}

	uint32_t getVertexSize() const
	{
		if (hasNormalMap)
			return (uint32_t)sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
		else
			return (uint32_t)sizeof(Vertex_POS_NOR_UV);
	}

	uint32_t getVertexSize(MeshFormat format) const
	{
		switch (format)
		{
		case MeshFormat::POS_NOR_UV:
			return (uint32_t)sizeof(Vertex_POS_NOR_UV);
		case MeshFormat::POS_NOR_UV_TAN_BITAN:
			return (uint32_t)sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
		}	
		return 0;
	}
	uint32_t getVertexCount() const
	{
		if (hasNormalMap)
			return (uint32_t)verticesTBN.size();
		else
			return (uint32_t)vertices.size();
	}
	uint32_t getVertexCount(MeshFormat format) const
	{
		switch (format)
		{
		case MeshFormat::POS_NOR_UV:
			return (uint32_t)vertices.size();
		case MeshFormat::POS_NOR_UV_TAN_BITAN:
			return (uint32_t)verticesTBN.size();
		}
		return 0;
	}
	uint32_t getIndicesCount() const
	{
		return (uint32_t)indices.size();
	}
	const unsigned int* getIndicesData() const
	{
		return indices.data();
	}
};


class AssimpLoader
{
public:

	AssimpLoader();
	~AssimpLoader();

	EngineMeshData loadStaticModel(const std::string& filePath);

private:
	std::vector<Vertex_POS_NOR_UV> m_vertices;
	std::vector<Vertex_POS_NOR_UV_TAN_BITAN> m_verticesTBN;
	std::vector<unsigned int> m_indices;
	std::vector<EngineMeshSubset> m_subsets;
	// Keep track of submesh offset in m_vertices and m_indices respectively
	unsigned int m_meshVertexCount;
	unsigned int m_meshIndexCount;

	bool m_hasNormalMap;

	SubMeshTree processNode(aiNode* node, const aiScene* scene, const std::string& path);
	EngineMeshSubset processMesh(aiMesh* mesh, const aiScene* scene, const std::string& path);
	Material GetPbrMaterials(aiMaterial* aiMat, const std::string& path);
};

