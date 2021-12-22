#pragma once
#pragma warning(push, 0)
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#pragma warning(pop)
#include <string>
#include <vector>
#include "GraphicsUtilityTypes.h"

enum class BlendMode
{
	opaque = 0,
	blend,
	mask,
};

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
	METALLICROUGHNESS = 1 << 11,
	ALBEDO_MAP = 1 << 12,
	PBR = 1 << 13,
	NO_BACKFACE_CULLING = 1 << 13,
	WIREFRAME = 1 << 15,

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




struct MetallicRoughnessMaterial
{
	MaterialProperties properties = MaterialProperties::NONE;
	BlendMode blendMode = BlendMode::opaque;
	float maskCutOfValue = 0;
	bool twoSided = false;
	std::string name = "";
	std::string baseColorPath = "";
	std::string normalPath = "";
	std::string metallicRoughnessPath = "";
	std::string emissivePath = "";
	std::string aoPath = "";

	float metallicFactor = 0;
	float roughnessFactor = 1;
	rfm::Vector4 baseColorFactor = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor = rfm::Vector4(1, 1, 1);
};

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
	std::string metallicroughnessPath = "";

};


struct EngineMeshSubset
{
	std::string name;
	//TextureTypes texTypes;
	EngineMaterial material;
	MetallicRoughnessMaterial pbrMaterial;
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
	MetallicRoughnessMaterial GetPbrMaterials(aiMaterial* aiMat, const std::string& path);
};

