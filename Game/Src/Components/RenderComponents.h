#pragma once
#include "rfEntity.hpp"
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include <memory>

struct DiffuseTexturMaterialComp : rfe::Component<DiffuseTexturMaterialComp>
{
	GID textureID;
};

struct IndexedMeshComp: rfe::Component<IndexedMeshComp>
{
	GID indexBuffer;
	GID vertexBuffer;
	/*std::shared_ptr<float> hej;*/
};