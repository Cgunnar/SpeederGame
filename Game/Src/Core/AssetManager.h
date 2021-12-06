#pragma once
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include "AssimpLoader.h"

enum class SimpleMesh
{
	Quad = 1,
};



class AssetManager
{
public:
	static void Init();
	static void Destroy();
	static AssetManager& Get();

	std::shared_ptr<Texture2D> GetTexture2D(GID guid) const;
	const SubMesh& GetMesh(MeshID id) const;
	const SubMesh& GetMesh(SimpleMesh mesh) const;
	GID AddTexture2D(std::shared_ptr<Texture2D> tempArgumentFixCreationOfTexture2dLater);
	MeshID AddMesh(SubMesh mesh);

	GID LoadModel(const std::string& filePath);

private:
	AssetManager();
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	AssimpLoader m_assimpLoader;
	std::unordered_map<uint64_t, std::shared_ptr<Texture2D>> m_textures;
	std::vector<SubMesh> m_meshes;
};

