#include "pch.hpp"
#include "TerrainLoader.h"


using namespace rfm;

TerrainLoader::~TerrainLoader()
{
    if (m_heightMapData) delete m_heightMapData;
    if (m_heightMap.vertices) delete m_heightMap.vertices;
}

void TerrainLoader::CreateTerrainFromBMP(const std::string& fileName, float scale)
{
#pragma warning(suppress : 4996)
    FILE* fp = fopen(fileName.c_str(), "rb");
    assert(fp);

    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);

    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    this->m_heightMap.width = infoHeader.biWidth;
    this->m_heightMap.height = infoHeader.biHeight;
    size_t size = infoHeader.biSizeImage;
    this->m_heightMapData = new uint8_t[size];

    fseek(fp, fileHeader.bfOffBits, SEEK_SET);
    fread(this->m_heightMapData, size, 1, fp);
    fclose(fp);



    //m_heightMap.vertices = new Vertex_POS_NOR_UV[m_heightMap.width * m_heightMap.height];

    uint8_t heightValue = 0;
    int offset = 0;
    int i = 0;

    for (int row = 0; row < m_heightMap.height; row++)
    {
        for (int col = 0; col < m_heightMap.width; col++)
        {
            heightValue = m_heightMapData[offset];
            i = (m_heightMap.width * row) + col;
            /*m_heightMap.vertices[i].position = rfm::Vector3(col, heightValue * scale, -row);
            m_heightMap.vertices[i].uv = rfm::Vector2(col, -row);*/
            Vertex_POS_NOR_UV v;
            
            v.position = rfm::Vector3(col, heightValue * scale, row);
            v.uv = rfm::Vector2(col, m_heightMap.height - row);
            m_vertices.push_back(v);
            offset += 4; //rgba
        }
    }

    CreateTerrain();
}

const std::vector<Vertex_POS_NOR_UV>& TerrainLoader::GetVertices() const
{
    return m_vertices;
}

const std::vector<uint32_t>& TerrainLoader::GetIndices() const
{
    return m_indices;
}

void TerrainLoader::CreateTerrain()
{
    int index;
    for (int row = 0; row < m_heightMap.height - 1; row++)
    {
        for (int col = 0; col < m_heightMap.width - 1; col++)
        {
            index = m_heightMap.width * row + col;

            //tri
            m_indices.push_back(index + 1);
            m_indices.push_back(index);
            m_indices.push_back(index + m_heightMap.width);

            //tri
            m_indices.push_back(index + m_heightMap.width + 1);
            m_indices.push_back(index + 1);
            m_indices.push_back(index + m_heightMap.width);
        }
    }

    assert((m_indices.size() % 3) == 0);
    for (size_t i = 0; i < m_indices.size(); i += 3)
    {
        auto& v0 = m_vertices[m_indices[i + 0]];
        auto& v1 = m_vertices[m_indices[i + 1]];
        auto& v2 = m_vertices[m_indices[i + 2]];

        Triangle tri;
        tri[0] = v0.position;
        tri[1] = v1.position;
        tri[2] = v2.position;
        CalcNormal(tri);

        v0.normal += tri.normal;
        v1.normal += tri.normal;
        v2.normal += tri.normal;

        m_triangles.push_back(tri);
    }
    for (auto i : m_indices)
    {
        m_vertices[i].normal.normalize();
    }
}

void TerrainLoader::CalcNormal(Triangle& tri) const
{
    tri.normal = cross(tri[1] - tri[0], tri[2] - tri[0]);
}


