#include "pch.hpp"
#include "TerrainMeshGenerator.h"
#include "Geometry.h"
#include "RimfrostMath.hpp"

using namespace rfm;

TerrainMeshGenerator::~TerrainMeshGenerator()
{
}

void TerrainMeshGenerator::CreateTerrainFromBMP(const std::string& fileName, float scale, rfm::Vector2 uvScale)
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

    CreateTerrain(scale, heightMapFloat.data(), infoHeader.biWidth, infoHeader.biHeight, uvScale);
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

void TerrainMeshGenerator::CreateTerrain(float scale, const float* hightMap, int width, int height, rfm::Vector2 uvScale)
{
    m_indices.clear();
    m_triangles.clear();
    m_vertices.clear();
    m_verticesTBN.clear();

    if (uvScale.x == 0) uvScale.x = static_cast<float>(width);
    if (uvScale.y == 0) uvScale.y = static_cast<float>(height);

    uint8_t heightValue = 0;
    int offset = 0;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int i = (width * row) + col;

            Vertex_POS_NOR_UV v;
            float x = static_cast<float>(col - static_cast<float>(width) / 2.0f);
            float y = static_cast<float>(hightMap[offset++] * scale);
            float z = static_cast<float>(row - static_cast<float>(height) / 2.0f);
            v.position = rfm::Vector3(x, y, z);
            v.uv = rfm::Vector2(static_cast<float>(col) / uvScale.x , static_cast<float>(row) / uvScale.y);
            m_vertices.push_back(v);


            Vertex_POS_NOR_UV_TAN_BITAN vTBN;
            vTBN.position = v.position;
            vTBN.uv = v.uv;
            m_verticesTBN.push_back(vTBN);
        }
    }



    int index;
    for (int row = 0; row < height - 1; row++)
    {
        for (int col = 0; col < width - 1; col++)
        {
            index = width * row + col;

            //tri
            m_indices.push_back(index + 1);
            m_indices.push_back(index);
            m_indices.push_back(index + width);

            //tri
            m_indices.push_back(index + width + 1);
            m_indices.push_back(index + 1);
            m_indices.push_back(index + width);
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


