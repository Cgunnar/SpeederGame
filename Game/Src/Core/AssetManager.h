#pragma once
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include "AssimpLoader.h"

enum class SimpleMesh
{
	Quad_POS_NOR_UV = 1,
	UVSphere_POS_NOR_UV = 2,
};





class AssetManager
{
public:
	static void Init();
	static void Destroy();
	static AssetManager& Get();

	std::shared_ptr<Texture2D> GetTexture2D(GID guid) const;
	const SubMesh& GetMesh(RenderUnitID id) const;
	const SubMesh& GetMesh(SimpleMesh mesh) const;
	const RenderUnit& GetRenderUnit(RenderUnitID id) const;
	RenderUnitID AddMesh(SubMesh mesh);
	RenderUnitID AddRenderUnit(const SubMesh& subMesh, const Material& material);
	RenderUnitID AddRenderUnit(RenderUnit renderUnit);

	GID LoadModel(const std::string& filePath);
	Model& GetModel(GID modelID);
	GID LoadTex2D(const std::string& path, LoadTexFlag flags);
private:
	AssetManager();
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	void TraverseSubMeshTree(SubMeshTree& subMeshTrees, SubModel& subModel, VertexBuffer vb, IndexBuffer ib);
	

	AssimpLoader m_assimpLoader;
	std::unordered_map<uint64_t, std::shared_ptr<Texture2D>> m_textures;
	std::unordered_map<uint64_t, Model> m_models;
	std::unordered_map<uint64_t, Material> m_materials;
	std::vector<RenderUnit> m_renderUnits;
	std::unordered_map<std::string, GID> m_filePathMap;
};

