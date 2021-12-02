#pragma once
#include "rfEntity.hpp"
#include "utilityTypes.h"
#include "GraphicsResources.h"

struct DiffuseTexturMaterialComp : rfe::Component<DiffuseTexturMaterialComp>
{
	GID textureID;
};

struct IndexedMeshComp: rfe::Component<IndexedMeshComp>
{
	IndexBuffer indexBuffer;
	VertexBuffer vertexBuffer;
};