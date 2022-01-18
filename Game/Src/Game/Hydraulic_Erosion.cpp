/*this function I have copied from Sebastion Lague
https://github.com/SebLague/Hydraulic-Erosion/blob/master/Assets/Scripts/Erosion.cs
MIT License
Copyright(c) 2019 Sebastian Lague
Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/


#include "pch.hpp"
#include "Hydraulic_Erosion.h"
#include "RimfrostMath.hpp"
#include "RandGen.hpp"
#include "TerrainMapGenerator.h"

using namespace rfm;

ErosionSimulator::BrushIndicesAndWeights ErosionSimulator::s_brush;


struct HeightAndGradient
{
	float height = 0;
	Vector2 grad;
};
HeightAndGradient CalculateHeightAndGradient(const std::vector<float>& nodes, Vector2 pos);
float CalculateHeight(const std::vector<float>& nodes, Vector2 pos);

constexpr float inertia = 0.05f;
constexpr float sedimentCapacityFactor = 4.0f;
constexpr float minSedimentCapacity = 0.01f;
constexpr float depositSpeed = 0.3f;
constexpr float erodeSpeed = 0.3f;
constexpr float evaporateSpeed = .01f;
constexpr float gravity = 4;

constexpr float initialWaterVolume = 1;
constexpr float initialSpeed = 1;
constexpr int maxDropletLifetime = 25;
constexpr int erosionRadius = 3;
constexpr unsigned int seed = 421212;
constexpr int MaxIterations = 40000;

void ErosionSimulator::Init(int radius)
{
	InitializeBrush(radius);
}

void ErosionSimulator::Erode(TerrainMap& map)
{
	assert(s_brush.initialized && chunkSize == map.width);
	std::uniform_real_distribution<> distr(0, chunkSize - 1);
	for (int iterations = 0; iterations < MaxIterations; iterations++)
	{
		Vector2 pos;
		std::ranlux48 engX(seed + iterations);
		pos.x = static_cast<float>(distr(engX));
		std::ranlux48 engY(10000*pos.x);
		pos.y = static_cast<float>(distr(engY));
		Vector2 direction;
		float speed = initialSpeed;
		float water = initialWaterVolume;
		float sediment = 0;


		for (int lifeTime = 0; lifeTime < maxDropletLifetime; lifeTime++)
		{
			Vector2I node = { static_cast<int>(pos.x), static_cast<int>(pos.y) };
			int dropIndex = node.y * chunkSize + node.x;
			Vector2 cellOffset = pos - node;

			HeightAndGradient hg = CalculateHeightAndGradient(map.heightMap, pos);

			direction = direction * inertia - hg.grad * (1.0f - inertia);
			direction.normalize();

			pos += direction;

			if ((abs(direction.x) < std::numeric_limits<float>::epsilon() && abs(direction.y) < std::numeric_limits<float>::epsilon()) || pos.x < 0.0f || pos.x >= map.width - 1.0f || pos.y < 0.0f || pos.y >= map.height - 1.0f)
			{
				break;
			}

			// Find the droplet's new height and calculate the deltaHeight
			float newHeight = CalculateHeight(map.heightMap, pos);
			float deltaHeight = newHeight - hg.height;

			// Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
			float sedimentCapacity = std::max(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity);
			// If carrying more sediment than capacity, or if flowing uphill:
			if (sediment > sedimentCapacity || deltaHeight > 0) {
				// If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
				float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
				sediment -= amountToDeposit;

				// Add the sediment to the four nodes of the current cell using bilinear interpolation
				// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
				map.heightMap[dropIndex] += amountToDeposit * (1 - cellOffset.x) * (1 - cellOffset.y);
				map.heightMap[dropIndex + 1] += amountToDeposit * cellOffset.x * (1 - cellOffset.y);
				map.heightMap[dropIndex + map.width] += amountToDeposit * (1 - cellOffset.x) * cellOffset.y;
				map.heightMap[dropIndex + map.width + 1] += amountToDeposit * cellOffset.x * cellOffset.y;
			}
			else {
				// Erode a fraction of the droplet's current carry capacity.
				// Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
				float amountToErode = std::min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);

				// Use erosion brush to erode from all nodes inside the droplet's erosion radius
				for (int brushPointIndex = 0; brushPointIndex < s_brush.indices[dropIndex].size(); brushPointIndex++) {
					int nodeIndex = s_brush.indices[dropIndex][brushPointIndex];
					float weighedErodeAmount = amountToErode * s_brush.weights[dropIndex][brushPointIndex];
					float deltaSediment = (map.heightMap[nodeIndex] < weighedErodeAmount) ? map.heightMap[nodeIndex] : weighedErodeAmount;
					map.heightMap[nodeIndex] -= deltaSediment;
					sediment += deltaSediment;
				}
			}

			// Update droplet's speed and water content
			speed = sqrt(std::max(0.0f, speed * speed + deltaHeight * gravity));
			water *= (1 - evaporateSpeed);
		}
	}
}


HeightAndGradient CalculateHeightAndGradient(const std::vector<float>& nodes, Vector2 pos) {
	int coordX = static_cast<int>(pos.x);
	int coordY = static_cast<int>(pos.y);

	// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
	float x = pos.x - coordX;
	float y = pos.y - coordY;

	// Calculate heights of the four nodes of the droplet's cell
	int nodeIndexNW = coordY * chunkSize + coordX;
	float heightNW = nodes[nodeIndexNW];
	float heightNE = nodes[nodeIndexNW + 1];
	float heightSW = nodes[nodeIndexNW + chunkSize];
	float heightSE = nodes[nodeIndexNW + chunkSize + 1];

	HeightAndGradient hg;
	// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges

	hg.grad.x = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
	hg.grad.y = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

	// Calculate height with bilinear interpolation of the heights of the nodes of the cell
	hg.height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

	return hg;
}

float CalculateHeight(const std::vector<float>& nodes, Vector2 pos)
{
	int coordX = static_cast<int>(pos.x);
	int coordY = static_cast<int>(pos.y);
	float x = pos.x - coordX;
	float y = pos.y - coordY;
	int nodeIndexNW = coordY * chunkSize + coordX;
	float heightNW = nodes[nodeIndexNW];
	float heightNE = nodes[nodeIndexNW + 1];
	float heightSW = nodes[nodeIndexNW + chunkSize];
	float heightSE = nodes[nodeIndexNW + chunkSize + 1];
	return heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;
}

void ErosionSimulator::InitializeBrush(int radius)
{
	s_brush.indices.resize(chunkSize * chunkSize);
	s_brush.weights.resize(chunkSize * chunkSize);

	std::vector<int> xOffsets(radius * radius * 4);
	std::vector<int> yOffsets(radius * radius * 4);
	std::vector<float> weights(radius * radius * 4);
	float weightSum = 0;
	int addIndex = 0;

	for (int i = 0; i < s_brush.indices.size(); i++) {
		int centreX = i % chunkSize;
		int centreY = i / chunkSize;

		if (centreY <= radius || centreY >= chunkSize - radius || centreX <= radius + 1 || centreX >= chunkSize - radius) {
			weightSum = 0;
			addIndex = 0;
			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					float sqrDst = x * x + y * y;
					if (sqrDst < radius * radius) {
						int coordX = centreX + x;
						int coordY = centreY + y;

						if (coordX >= 0 && coordX < chunkSize && coordY >= 0 && coordY < chunkSize) {
							float weight = 1 - sqrt(sqrDst) / radius;
							weightSum += weight;
							weights[addIndex] = weight;
							xOffsets[addIndex] = x;
							yOffsets[addIndex] = y;
							addIndex++;
						}
					}
				}
			}
		}

		int numEntries = addIndex;
		s_brush.indices[i] = std::vector<int>(numEntries);
		s_brush.weights[i] = std::vector<float>(numEntries);

		for (int j = 0; j < numEntries; j++) {
			s_brush.indices[i][j] = (yOffsets[j] + centreY) * chunkSize + xOffsets[j] + centreX;
			s_brush.weights[i][j] = weights[j] / weightSum;
		}
	}
	s_brush.initialized = true;
}