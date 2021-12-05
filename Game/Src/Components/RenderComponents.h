#pragma once
#include "rfEntity.hpp"
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"

struct DiffuseTexturMaterialComp : rfe::Component<DiffuseTexturMaterialComp>
{
	GID textureID;
	rfm::Vector3 specularColor{ 1,1,1 };
	float shininess = 700;
};

struct IndexedMeshComp: rfe::Component<IndexedMeshComp>
{
	IndexBuffer indexBuffer;
	VertexBuffer vertexBuffer;
};



struct PointLightComp : rfe::Component<PointLightComp>
{
	PointLight pointLight;
};