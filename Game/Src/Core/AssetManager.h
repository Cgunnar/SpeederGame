#pragma once
#include "utilityTypes.h"
#include "GraphicsResources.h"

class AssetManager
{
public:
	static void Init();
	static void Destroy();
	static AssetManager& Get();

	std::shared_ptr<Texture2D> GetTexture2D(GID guid) const;
	GID AddTexture2D(std::shared_ptr<Texture2D> tempArgumentFixCreationOfTexture2dLater);


private:
	AssetManager();
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	std::unordered_map<uint64_t, std::shared_ptr<Texture2D>> m_textures;
};

