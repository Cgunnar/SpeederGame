#pragma once
#include <variant>
#include "RimfrostMath.hpp"
#include "utilityTypes.h"


struct PhongMaterial_Color
{
	//linear color space
	rfm::Vector3 ambientColor{ 0.8f, 0.8f, 0.8f };
	rfm::Vector3 diffuseColor{ 0.8f, 0.8f, 0.8f };
	rfm::Vector3 specularColor{ 1, 1, 1 };
	float shininess = 800;
};

struct PhongMaterial_DiffTex
{
	GID diffuseTextureID;
	rfm::Vector3 specularColor{ 1, 1, 1 };
	float shininess = 800;
};

struct Material
{
	std::string name;
	enum class Type
	{
		PhongMaterial_Color = 0,
		PhongMaterial_DiffTex,
	} type;
	std::variant <
		PhongMaterial_Color, PhongMaterial_DiffTex
	> materialVariant;
};

