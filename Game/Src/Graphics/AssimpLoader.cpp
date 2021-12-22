#include "pch.hpp"
#include "AssimpLoader.h"
#include <assimp/pbrmaterial.h>

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
	//assert(modelGraph.subMeshes.empty()); // i want to know if this can be filled or if the root always is empty



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

	MetallicRoughnessMaterial pbrMaterial = GetPbrMaterials(mtl, path);

	aiString diffName, normName, specName, metalRougName, albedoName;
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

	subsetData.name = materialName.C_Str();
	subsetData.material = newMat;
	subsetData.pbrMaterial = pbrMaterial;

	subsetData.vertexCount = mesh->mNumVertices;
	subsetData.vertexStart = m_meshVertexCount;
	m_meshVertexCount += mesh->mNumVertices;

	subsetData.indexCount = indicesThisMesh;
	subsetData.indexStart = m_meshIndexCount;
	m_meshIndexCount += indicesThisMesh;

	/*m_subsets.push_back(subsetData);*/
	return subsetData;
}

MetallicRoughnessMaterial AssimpLoader::GetPbrMaterials(aiMaterial* aiMat, const std::string& path)
{
	MetallicRoughnessMaterial pbrMat;

	aiString materialName;
	if (!aiMat->Get(AI_MATKEY_NAME, materialName))
	{
		pbrMat.name = materialName.C_Str();
	}

	bool twoSided = false;
	if (!aiMat->Get(AI_MATKEY_TWOSIDED, twoSided))
	{
		pbrMat.twoSided = twoSided;
		pbrMat.properties = pbrMat.properties | MaterialProperties::NO_BACKFACE_CULLING;
	}

	float metallicFactor;
	if (!aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor))
	{
		pbrMat.metallicFactor = metallicFactor;
	}

	float roughnessFactor;
	if (!aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor))
	{
		pbrMat.roughnessFactor = roughnessFactor;
		pbrMat.properties = pbrMat.properties | MaterialProperties::PBR;
	}

	aiString alphaMode;
	if (!aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode))
	{
		if (alphaMode == aiString("MASK"))
		{
			pbrMat.blendMode = BlendMode::mask;
			float cutOf = 1;
			if (!aiMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, cutOf))
				pbrMat.maskCutOfValue = cutOf;
			pbrMat.properties = pbrMat.properties | MaterialProperties::ALPHA_TESTING;
		}
		else if(alphaMode == aiString("BLEND"))
		{
			pbrMat.blendMode = BlendMode::blend;
			pbrMat.properties = pbrMat.properties | MaterialProperties::ALPHA_BLENDING;
		}
		else if (alphaMode == aiString("OPAQUE"))
		{
			pbrMat.blendMode = BlendMode::opaque;
		}
	}

	
	aiColor4D baseColorFactor(0.0f, 0.0f, 0.0f, 0.0f);
	if (!aiMat->Get(AI_MATKEY_BASE_COLOR, baseColorFactor))
	{
		pbrMat.baseColorFactor = rfm::Vector4(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3]);
	}

	bool use_metallic_map;
	if (!aiMat->Get(AI_MATKEY_USE_METALLIC_MAP, use_metallic_map))
	{
		assert(false); // is this used?
	}
	bool use_roughness_map;
	if (!aiMat->Get(AI_MATKEY_USE_ROUGHNESS_MAP, use_roughness_map))
	{
		assert(false); // is this used?
	}

	aiColor3D ai_matkey_emissive(0.0f, 0.0f, 0.0f);
	if (!aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, ai_matkey_emissive))
	{
		pbrMat.emissiveFactor = rfm::Vector3(ai_matkey_emissive[0], ai_matkey_emissive[1], ai_matkey_emissive[2]);
	}

	aiString baseColorName, normName, metallicRoughnessName, emissiveName, aoName;
	if (!aiMat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &baseColorName))
	{
		pbrMat.baseColorPath = path + baseColorName.C_Str();
		pbrMat.properties = pbrMat.properties | MaterialProperties::ALBEDO_MAP;
	}
	
	if (!aiMat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessName))
	{
		pbrMat.metallicRoughnessPath = path + metallicRoughnessName.C_Str();
		pbrMat.properties = pbrMat.properties | MaterialProperties::METALLICROUGHNESS;
	}
	
	if (!aiMat->GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &emissiveName))
	{
		pbrMat.emissivePath = path + emissiveName.C_Str();
		pbrMat.properties = pbrMat.properties | MaterialProperties::IS_EMISSIVE;
	}

	if (!aiMat->GetTexture(aiTextureType_NORMALS, 0, &normName))
	{
		pbrMat.normalPath = path + normName.C_Str();
		m_hasNormalMap = true; // to use tangent and bitangent vertexbuffer
		pbrMat.properties = pbrMat.properties | MaterialProperties::NORMAL_MAP;
	}

	if (!aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aoName))
	{
		pbrMat.aoPath = path + aoName.C_Str();
		assert(false);// want to know if this is used
	}

	

	return pbrMat;
}
