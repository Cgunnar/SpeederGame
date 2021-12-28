#include "pch.hpp"
#include "Geometry.h"
#include <DirectXMath.h>

namespace Geometry
{
	using namespace DirectX;
	using namespace rfm;
	void CalcTanAndBiTan(std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices);


	//from directXTK
	Sphere_POS_NOR_UV::Sphere_POS_NOR_UV(int tessellation, float radius)
	{
		assert(tessellation >= 3);
		int verticalSegments = tessellation;
		int horizontalSegments = tessellation * 2;

		// Create rings of vertices at progressively higher latitudes.
		for (int i = 0; i <= verticalSegments; i++)
		{
			float v = 1 - float(i) / float(verticalSegments);
			float latitude = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
			float dy, dxz;
			XMScalarSinCos(&dy, &dxz, latitude);
			// Create a single ring of vertices at this latitude.
			for (int j = 0; j <= horizontalSegments; j++)
			{
				float u = float(j) / float(horizontalSegments);
				float longitude = float(j) * XM_2PI / float(horizontalSegments);
				float dx, dz;
				XMScalarSinCos(&dx, &dz, longitude);
				dx *= dxz;
				dz *= dxz;
				rfm::Vector3 normal = {dx, dy, dz};
				rfm::Vector3 position = radius * normal;
				rfm::Vector2 textureCoordinate = { u, v };
				vertices.push_back({ position, normal, textureCoordinate });
			}
		}

		// Fill the index buffer with triangles joining each pair of latitude rings.
		int stride = horizontalSegments + 1;
		for (int i = 0; i < verticalSegments; i++)
		{
			for (int j = 0; j <= horizontalSegments; j++)
			{
				int nextI = i + 1;
				int nextJ = (j + 1) % stride;
				indices.push_back(i * stride + j);
				indices.push_back(nextI * stride + j);
				indices.push_back(i * stride + nextJ);

				indices.push_back(i * stride + nextJ);
				indices.push_back(nextI * stride + j);
				indices.push_back(nextI * stride + nextJ);
			}
		}

		assert((indices.size() % 3) == 0);
		for (auto it = indices.begin(); it != indices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}
		for (auto& it : vertices)
		{
			it.uv.x = (1.f - it.uv.x);
		}
	}



	Sphere_POS_NOR_UV_TAN_BITAN::Sphere_POS_NOR_UV_TAN_BITAN(int tessellation, float radius)
	{
		assert(tessellation >= 3);
		int verticalSegments = tessellation;
		int horizontalSegments = tessellation * 2;

		// Create rings of vertices at progressively higher latitudes.
		for (int i = 0; i <= verticalSegments; i++)
		{
			float v = 1 - float(i) / float(verticalSegments);
			float latitude = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
			float dy, dxz;
			XMScalarSinCos(&dy, &dxz, latitude);
			// Create a single ring of vertices at this latitude.
			for (int j = 0; j <= horizontalSegments; j++)
			{
				float u = float(j) / float(horizontalSegments);
				float longitude = float(j) * XM_2PI / float(horizontalSegments);
				float dx, dz;
				XMScalarSinCos(&dx, &dz, longitude);
				dx *= dxz;
				dz *= dxz;
				rfm::Vector3 normal = { dx, dy, dz };
				/*normal.normalize();
				rfm::Vector3 tan;
				rfm::Vector3 biTan;
				CalcTanAndBiTanFromNormal(normal, tan, biTan);*/
				
				rfm::Vector3 position = radius * normal;
				rfm::Vector2 textureCoordinate = { u, v };
				//vertices.push_back({ position, normal, textureCoordinate, biTan, tan });
				vertices.push_back({ position, normal, textureCoordinate, Vector3(), Vector3() });
			}
		}

		// Fill the index buffer with triangles joining each pair of latitude rings.
		int stride = horizontalSegments + 1;
		for (int i = 0; i < verticalSegments; i++)
		{
			for (int j = 0; j <= horizontalSegments; j++)
			{
				int nextI = i + 1;
				int nextJ = (j + 1) % stride;
				indices.push_back(i * stride + j);
				indices.push_back(nextI * stride + j);
				indices.push_back(i * stride + nextJ);

				indices.push_back(i * stride + nextJ);
				indices.push_back(nextI * stride + j);
				indices.push_back(nextI * stride + nextJ);
			}
		}

		assert((indices.size() % 3) == 0);
		for (auto it = indices.begin(); it != indices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}
		for (auto& it : vertices)
		{
			it.uv.x = (1.f - it.uv.x);
		}

		CalcTanAndBiTan(vertices, indices);


	}
	void CalcTanAndBiTan(std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices)
	{
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			auto& v0 = vertices[indices[i + 0]];
			auto& v1 = vertices[indices[i + 1]];
			auto& v2 = vertices[indices[i + 2]];

			auto dP1 = v1.position - v0.position;
			auto dP2 = v2.position - v0.position;

			auto dUV1 = v1.uv - v0.uv;
			auto dUV2 = v2.uv - v0.uv;

			/*float r = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
			Vector3 tangent = (dP1 * dUV2.y - dP2 * dUV1.y) * r;
			Vector3 bitangent = (dP2 * dUV1.x - dP1 * dUV2.x) * r;*/

			float r = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
			Vector3 tangent = (dP1 * dUV2.y - dP2 * dUV1.y) * r;
			Vector3 bitangent = -(dP2 * dUV1.x - dP1 * dUV2.x) * r;



			v0.tangent += tangent;
			v0.biTangent += bitangent;
			v1.tangent += tangent;
			v1.biTangent += bitangent;
			v2.tangent += tangent;
			v2.biTangent += bitangent;
		}
		for (auto& v : vertices)
		{
			v.biTangent.normalize();
			v.tangent.normalize();
		}
	}
}


