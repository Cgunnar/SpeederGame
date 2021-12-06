#pragma once
#pragma warning(push, 0)
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#pragma warning(pop)
#include <string>

#include "MeshStructures.hpp"
#include "GraphicsUtilityTypes.h"


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
};

