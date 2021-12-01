#include "pch.hpp"
#include "AssetManager.h"

AssetManager* AssetManager::s_instance = nullptr;

void AssetManager::Init()
{
	assert(!s_instance);
	s_instance = new AssetManager();
}

void AssetManager::Destroy()
{
	assert(s_instance);
	delete s_instance;
	s_instance = nullptr;
}

AssetManager& AssetManager::Get()
{
	assert(s_instance);
	return *s_instance;
}

std::shared_ptr<Texture2D> AssetManager::GetTexture2D(GID guid) const
{
	if (auto it = m_textures.find(guid); it != m_textures.end())
	{
		return it->second;
	}
	return nullptr;
}

GID AssetManager::AddTexture2D(std::shared_ptr<Texture2D> tempArgumentFixCreationOfTexture2dLater)
{
	GID id = GID::GenerateNew();
	m_textures[id] = tempArgumentFixCreationOfTexture2dLater;
	return id;
}

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}
