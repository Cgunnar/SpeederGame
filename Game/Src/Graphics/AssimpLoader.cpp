#include "pch.hpp"
#include "AssimpLoader.h"

#ifdef DEBUG
    #pragma comment(lib, "assimp-vc142-mtd.lib")
#else
    #pragma comment(lib, "assimp-vc142-mt.lib")
#endif // DEBUG


AssimpLoader::AssimpLoader() :
	m_meshVertexCount(0),
	m_meshIndexCount(0),
	m_hasNormalMap(false)
{
}

AssimpLoader::~AssimpLoader()
{
}

EngineMeshData AssimpLoader::loadStaticModel(const std::string& filePath)
{
	m_hasNormalMap = false;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		filePath,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace
		//|
		//aiProcess_DropNormals		// Added 15/03/2021
	);

	if (scene == nullptr)
	{
		OutputDebugStringW(L"Assimp: File not found!");
		assert(false);
	}

	std::string path = "";
	if (filePath.rfind('/') != std::string::npos)
	{
		size_t pos = filePath.rfind('/');
		path = filePath.substr(0, pos + 1);
	}

	unsigned int totalVertexCount = 0;
	unsigned int totalSubsetCount = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		totalVertexCount += scene->mMeshes[i]->mNumVertices;
		++totalSubsetCount;
	}
	m_vertices.reserve(totalVertexCount);
	m_verticesTBN.reserve(totalVertexCount);
	m_indices.reserve(totalVertexCount);
	m_subsets.reserve(totalSubsetCount);

	SubMeshTree modelGraph;

	modelGraph = processNode(scene->mRootNode, scene, path);
	assert(modelGraph.subMeshes.empty()); // i want to know if this can be filled or if the root always is empty



	EngineMeshData data;
	data.indices = m_indices;
	data.vertices = m_vertices;
	data.verticesTBN = m_verticesTBN;
	data.subMeshesVector = m_subsets;
	data.subsetsInfo = modelGraph;

	m_indices.clear();
	m_vertices.clear();
	m_verticesTBN.clear();
	m_subsets.clear();
	m_meshVertexCount = 0;
	m_meshIndexCount = 0;



	data.hasNormalMap = m_hasNormalMap;

	return data;
}

SubMeshTree AssimpLoader::processNode(aiNode* node, const aiScene* scene, const std::string& path)
{
	SubMeshTree meshTree;

	// For each mesh in the node, process it!
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		EngineMeshSubset subMesh = processMesh(mesh, scene, path);
		meshTree.subMeshes.push_back(subMesh);
		m_subsets.push_back(subMesh);
		meshTree.subMeshesIndex.push_back(m_subsets.size() - 1);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		meshTree.nodes.push_back(processNode(node->mChildren[i], scene, path));
	}
	return meshTree;
}

// Subset of Mesh
EngineMeshSubset AssimpLoader::processMesh(aiMesh* mesh, const aiScene* scene, const std::string& path)
{

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex_POS_NOR_UV_TAN_BITAN vertTBN = { };
		vertTBN.biTangent.x = mesh->mBitangents[i].x;
		vertTBN.biTangent.y = mesh->mBitangents[i].y;
		vertTBN.biTangent.z = mesh->mBitangents[i].z;

		vertTBN.tangent.x = mesh->mTangents[i].x;
		vertTBN.tangent.y = mesh->mTangents[i].y;
		vertTBN.tangent.z = mesh->mTangents[i].z;

		vertTBN.position.x = mesh->mVertices[i].x;
		vertTBN.position.y = mesh->mVertices[i].y;
		vertTBN.position.z = mesh->mVertices[i].z;

		vertTBN.normal.x = mesh->mNormals[i].x;
		vertTBN.normal.y = mesh->mNormals[i].y;
		vertTBN.normal.z = mesh->mNormals[i].z;

		Vertex_POS_NOR_UV vert = { };
		vert.position.x = mesh->mVertices[i].x;
		vert.position.y = mesh->mVertices[i].y;
		vert.position.z = mesh->mVertices[i].z;

		vert.normal.x = mesh->mNormals[i].x;
		vert.normal.y = mesh->mNormals[i].y;
		vert.normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0])
		{
			vert.uv.x = mesh->mTextureCoords[0][i].x;
			vert.uv.y = mesh->mTextureCoords[0][i].y;

			vertTBN.uv.x = mesh->mTextureCoords[0][i].x;
			vertTBN.uv.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vert.uv.x = 0.0f;
			vert.uv.y = 0.0f;

			vertTBN.uv.x = 0.0f;
			vertTBN.uv.y = 0.0f;
		}

		m_vertices.push_back(vert);
		m_verticesTBN.push_back(vertTBN);
	}

	unsigned int indicesThisMesh = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			m_indices.push_back(face.mIndices[j]);
			++indicesThisMesh;
		}

	}

	EngineMaterial newMat;
	aiReturn errorCheck;

	// Get material
	auto mtl = scene->mMaterials[mesh->mMaterialIndex];
	aiString diffName, normName, specName;
	//mtl->GetTexture(aiTextureType_HEIGHT, 0, &normName);
	if (!mtl->GetTexture(aiTextureType_DIFFUSE, 0, &diffName))
	{
		newMat.properties = newMat.properties | MaterialProperties::DIFFUSE_MAP;
		newMat.diffuseMapPath = path + diffName.C_Str();
	}
	if (!mtl->GetTexture(aiTextureType_NORMALS, 0, &normName))
	{
		newMat.properties = newMat.properties | MaterialProperties::NORMAL_MAP;
		newMat.normalMapPath = path + normName.C_Str();
		m_hasNormalMap = true;
	}
	if(!mtl->GetTexture(aiTextureType_SPECULAR, 0, &specName))
	{
		newMat.properties = newMat.properties | MaterialProperties::SPECULAR_MAP;
		newMat.specularMapPath = path + specName.C_Str();
	}

	aiString materialName;
	if (!mtl->Get(AI_MATKEY_NAME, materialName))
	{
		newMat.name = materialName.C_Str();
	}

	aiColor3D colorDiff(0.f, 0.f, 0.f);
	if (!mtl->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiff))
	{
		newMat.properties = newMat.properties | MaterialProperties::DIFFUSE_COLOR;
		newMat.colorDiffuse = rfm::Vector3(colorDiff[0], colorDiff[1], colorDiff[2]);
	}

	aiColor3D colorSpec(0.f, 0.f, 0.f);
	if (!mtl->Get(AI_MATKEY_COLOR_SPECULAR, colorSpec))
	{
		newMat.properties = newMat.properties | MaterialProperties::SPECULAR_COLOR;
		newMat.colorSpecular = rfm::Vector3(colorSpec[0], colorSpec[1], colorSpec[2]);
	}

	aiColor3D colorAmbi(0.f, 0.f, 0.f);
	if (!mtl->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbi))
	{
		newMat.properties = newMat.properties | MaterialProperties::AMBIENT_COLOR;
		newMat.colorAmbient = rfm::Vector3(colorAmbi[0], colorAmbi[1], colorAmbi[2]);
	}


	float shininess;
	if (!mtl->Get(AI_MATKEY_SHININESS, shininess))
	{
		newMat.properties = newMat.properties | MaterialProperties::SHININESS;
		newMat.shininess = shininess;
	}

	float opacity;
	if (!mtl->Get(AI_MATKEY_OPACITY, opacity))
	{
		newMat.opacity = 1;
		if (opacity < 1)
		{
			newMat.properties = newMat.properties | MaterialProperties::ALPHA_BLENDING_CONSTANS_OPACITY;
			newMat.opacity = opacity;
		}
	}

	// Subset data
	EngineMeshSubset subsetData = { };
	//subsetData.material = newMat;

	subsetData.name = materialName.C_Str();
	subsetData.material = newMat;
	/*subsetData.specularFileName = specName.C_Str();
	subsetData.diffuseFileName = diffName.C_Str();
	subsetData.color[0] = colorDiff[0];
	subsetData.color[1] = colorDiff[1];
	subsetData.color[2] = colorDiff[2];
	subsetData.normalFileName = normName.C_Str();*/

	//subsetData.filePath = path;

	subsetData.vertexCount = mesh->mNumVertices;
	subsetData.vertexStart = m_meshVertexCount;
	m_meshVertexCount += mesh->mNumVertices;

	subsetData.indexCount = indicesThisMesh;
	subsetData.indexStart = m_meshIndexCount;
	m_meshIndexCount += indicesThisMesh;

	/*m_subsets.push_back(subsetData);*/
	return subsetData;
}
