#pragma once
#include <string>
#include "assimp\Importer.hpp"

struct Model
{
	int a;
};

class AssimpLoader
{
public:
	AssimpLoader() = default;
	~AssimpLoader() = default;

	static Model Load(const std::string& path);
	
};

