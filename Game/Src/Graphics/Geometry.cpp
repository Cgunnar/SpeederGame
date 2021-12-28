#include "pch.hpp"
#include "Geometry.h"
#include <DirectXMath.h>
using namespace DirectX;

namespace Geometry
{

	//from directXTK
	Sphere_POS_NOR_UV::Sphere_POS_NOR_UV(int tessellation, float radius)
	{
		vertices.clear();
		indices.clear();

		assert(tessellation < 3);

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
				rfm::Vector3 textureCoordinate = { u, v, 0 };

				vertices.push_back({ position, -normal, textureCoordinate });
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
}


