#include "pch.hpp"
#include "TerrainMeshGenerator.h"
#include "Geometry.h"
#include "RimfrostMath.hpp"
#include "WorkerThreads.h"

using namespace rfm;

TerrainMesh TerrainMeshGenerator::CreateTerrainMeshFromBMP(const std::string& fileName, TerrainMeshDesc desc)
{
#pragma warning(suppress : 4996)
    FILE* fp = fopen(fileName.c_str(), "rb");
    assert(fp);

    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);

    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    size_t size = infoHeader.biSizeImage;
    uint8_t* bmpData  = new uint8_t[size];

    fseek(fp, fileHeader.bfOffBits, SEEK_SET);
    fread(bmpData, size, 1, fp);
    fclose(fp);

    std::vector<float> heightMapFloat;
    assert(size % 4 == 0);
    heightMapFloat.reserve(size / 4);
    int count = 0;

    for (int i = 0; i < size; i+=4)
    {
        heightMapFloat.push_back(rfm::InvLerp(0, 255, bmpData[i]));
    }
    delete[] bmpData;

    return CreateTerrainMeshFromHeightMapMemory(heightMapFloat.data(),
        infoHeader.biWidth, infoHeader.biHeight, desc);
}

TerrainMesh TerrainMeshGenerator::CreateTerrainMesh(const TerrainMap& terrainMap, TerrainMeshDesc desc)
{
    return CreateTerrainMeshFromHeightMapMemory(terrainMap.heightMap.data(),
        terrainMap.width, terrainMap.height, desc);
}

void TerrainMeshGenerator::AsyncCreateTerrainMeshInternal(const TerrainMap& terrainMap, std::function<void(TerrainMesh)> callback, TerrainMeshDesc desc)
{
    auto t = TerrainMeshGenerator::CreateTerrainMesh(terrainMap, desc);
    callback(t);
}

void TerrainMeshGenerator::AsyncCreateTerrainMesh(const TerrainMap& terrainMap, std::function<void(TerrainMesh)> callback, TerrainMeshDesc desc)
{
    WorkerThreads::AddTask(TerrainMeshGenerator::AsyncCreateTerrainMeshInternal, terrainMap, callback, desc);
}

TerrainMesh TerrainMeshGenerator::CreateTerrainMeshFromHeightMapMemory(const float* hightMap, int width, int height,
    TerrainMeshDesc desc)
{
    assert(0 <= desc.LOD && desc.LOD <= 6);
    TerrainMesh mesh;
    desc.LOD *= 2;
    if (!desc.LOD) desc.LOD = 1;
    int verticesPerLine = (width - 1) / desc.LOD + 1;
    int numTriangles = (verticesPerLine - 1) * (verticesPerLine - 1) * 2;

    mesh.indices.clear();
    mesh.indices.reserve((size_t)numTriangles * 3);
    mesh.triangles.clear();
    mesh.triangles.reserve(numTriangles);
    mesh.vertices.clear();
    mesh.vertices.reserve(verticesPerLine * (size_t)verticesPerLine);
    mesh.verticesTBN.clear();
    mesh.verticesTBN.reserve(verticesPerLine * (size_t)verticesPerLine);

    if (desc.uvScale.x == 0) desc.uvScale.x = static_cast<float>(width);
    if (desc.uvScale.y == 0) desc.uvScale.y = static_cast<float>(height);
    
    int index = 0;
    for (int y = 0; y < height; y+= desc.LOD)
    {
        for (int x = 0; x < width; x+= desc.LOD)
        {

            Vertex_POS_NOR_UV v;
            v.position.x = static_cast<float>(x) - static_cast<float>(width-1) / 2.0f;
            v.position.y = desc.heightScaleFunc(hightMap[y * width + x], desc.funktionParmK) * desc.heightScale;
            v.position.z = static_cast<float>(height-1) / 2.0f - static_cast<float>(y);
            v.uv = rfm::Vector2(static_cast<float>(x) / desc.uvScale.x , static_cast<float>(y) / desc.uvScale.y);
            mesh.vertices.push_back(v);


            Vertex_POS_NOR_UV_TAN_BITAN vTBN;
            vTBN.position = v.position;
            vTBN.uv = v.uv;
            mesh.verticesTBN.push_back(vTBN);


            if (x < width - 1 && y < height - 1)
            {
                //tri
                mesh.indices.push_back(index);
                mesh.indices.push_back(index + 1);
                mesh.indices.push_back(index + verticesPerLine);

                //tri
                mesh.indices.push_back(index + 1);
                mesh.indices.push_back(index + verticesPerLine + 1);
                mesh.indices.push_back(index + verticesPerLine);
            }
            index++;
        }
    }

    assert((mesh.indices.size() % 3) == 0);
    for (size_t i = 0; i < mesh.indices.size(); i += 3)
    {
        auto& v0 = mesh.vertices[mesh.indices[i + 0]];
        auto& v1 = mesh.vertices[mesh.indices[i + 1]];
        auto& v2 = mesh.vertices[mesh.indices[i + 2]];

        auto& v0TBN = mesh.verticesTBN[mesh.indices[i + 0]];
        auto& v1TBN = mesh.verticesTBN[mesh.indices[i + 1]];
        auto& v2TBN = mesh.verticesTBN[mesh.indices[i + 2]];

        Triangle tri;
        tri[0] = v0.position;
        tri[1] = v1.position;
        tri[2] = v2.position;
        CalcNormal(tri);

        v0.normal += tri.normal;
        v1.normal += tri.normal;
        v2.normal += tri.normal;

        v0TBN.normal += tri.normal;
        v1TBN.normal += tri.normal;
        v2TBN.normal += tri.normal;

        mesh.triangles.push_back(tri);
    }
    for (auto i : mesh.indices)
    {
        mesh.vertices[i].normal.normalize();
        mesh.verticesTBN[i].normal.normalize();
    }

    Geometry::CalcTanAndBiTan(mesh.verticesTBN, mesh.indices);
    return mesh;
}

void TerrainMeshGenerator::CalcNormal(Triangle& tri)
{
    tri.normal = cross(tri[1] - tri[0], tri[2] - tri[0]);
}




