#include "pch.hpp"
#include "AssimpLoader.h"

#ifdef DEBUG
    #pragma comment(lib, "assimp-vc142-mtd.lib")
#else
    #pragma comment(lib, "assimp-vc142-mt.lib")
#endif // DEBUG



Model AssimpLoader::Load(const std::string& path)
{
    Assimp::Importer importer;
    return Model();
}
