#include "pch.hpp"
#include "TerrainMeshGenerator.h"
#include "Geometry.h"
#include "RimfrostMath.hpp"

using namespace rfm;

TerrainMeshGenerator::~TerrainMeshGenerator()
{
}

void TerrainMeshGenerator::CreateTerrainMeshFromBMP(const std::string& fileName, float scale, int LOD, rfm::Vector2 uvScale)
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

    CreateTerrainMeshFromHeightMapMemory(heightMapFloat.data(), infoHeader.biWidth, infoHeader.biHeight,
        scale, LOD, uvScale);
}

const std::vector<Vertex_POS_NOR_UV>& TerrainMeshGenerator::GetVertices() const
{
    return m_vertices;
}

const std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& TerrainMeshGenerator::GetVerticesTBN() const
{
    return m_verticesTBN;
}

const std::vector<uint32_t>& TerrainMeshGenerator::GetIndices() const
{
    return m_indices;
}

void TerrainMeshGenerator::CreateTerrainMesh(TerrainMap terrainMap, float scale, int LOD,
    rfm::Vector2 uvScale, std::function<float(float)> heightScaleFunc)
{
    CreateTerrainMeshFromHeightMapMemory(terrainMap.heightMap.data(), terrainMap.width, terrainMap.height,
        scale, LOD, uvScale, heightScaleFunc);
}

void TerrainMeshGenerator::CreateTerrainMeshFromHeightMapMemory(const float* hightMap, int width, int height,
    float scale, int LOD, rfm::Vector2 uvScale, std::function<float(float)> heightScaleFunc)
{
    assert(0 <= LOD && LOD <= 6);
    LOD *= 2;
    if (!LOD) LOD = 1;
    int verticesPerLine = (width - 1) / LOD + 1;
    int numTriangles = (verticesPerLine - 1) * (verticesPerLine - 1) * 2;

    m_indices.clear();
    m_indices.reserve(numTriangles * 3);
    m_triangles.clear();
    m_triangles.reserve(numTriangles);
    m_vertices.clear();
    m_vertices.reserve(verticesPerLine * verticesPerLine);
    m_verticesTBN.clear();
    m_verticesTBN.reserve(verticesPerLine * verticesPerLine);

    if (uvScale.x == 0) uvScale.x = static_cast<float>(width);
    if (uvScale.y == 0) uvScale.y = static_cast<float>(height);
    
    int index = 0;
    for (int y = 0; y < height; y+=LOD)
    {
        for (int x = 0; x < width; x+=LOD)
        {

            Vertex_POS_NOR_UV v;
            v.position.x = static_cast<float>(x) - static_cast<float>(width-1) / 2.0f;
            v.position.y = heightScaleFunc(hightMap[y * width + x]) * scale;
            v.position.z = static_cast<float>(height-1) / 2.0f - static_cast<float>(y);
            v.uv = rfm::Vector2(static_cast<float>(x) / uvScale.x , static_cast<float>(y) / uvScale.y);
            m_vertices.push_back(v);


            Vertex_POS_NOR_UV_TAN_BITAN vTBN;
            vTBN.position = v.position;
            vTBN.uv = v.uv;
            m_verticesTBN.push_back(vTBN);


            if (x < width - 1 && y < height - 1)
            {
                //tri
                m_indices.push_back(index);
                m_indices.push_back(index + 1);
                m_indices.push_back(index + verticesPerLine);

                //tri
                m_indices.push_back(index + 1);
                m_indices.push_back(index + verticesPerLine + 1);
                m_indices.push_back(index + verticesPerLine);
            }
            index++;
        }
    }

    assert((m_indices.size() % 3) == 0);
    for (size_t i = 0; i < m_indices.size(); i += 3)
    {
        auto& v0 = m_vertices[m_indices[i + 0]];
        auto& v1 = m_vertices[m_indices[i + 1]];
        auto& v2 = m_vertices[m_indices[i + 2]];

        auto& v0TBN = m_verticesTBN[m_indices[i + 0]];
        auto& v1TBN = m_verticesTBN[m_indices[i + 1]];
        auto& v2TBN = m_verticesTBN[m_indices[i + 2]];

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

        m_triangles.push_back(tri);
    }
    for (auto i : m_indices)
    {
        m_vertices[i].normal.normalize();
        m_verticesTBN[i].normal.normalize();
    }

    Geometry::CalcTanAndBiTan(m_verticesTBN, m_indices);
}

void TerrainMeshGenerator::CalcNormal(Triangle& tri) const
{
    tri.normal = cross(tri[1] - tri[0], tri[2] - tri[0]);
}


