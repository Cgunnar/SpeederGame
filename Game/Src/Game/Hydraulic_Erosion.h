#pragma once
#include "TerrainTypes.h"
#include "RimfrostMath.hpp"
struct HeightAndGradient
{
	float height = 0;
	rfm::Vector2 grad;
};

class ErosionSimulator
{
public:
	static void Init(int radius);
	static void Erode(TerrainMap& map, int maxIterations);
	static HeightAndGradient CalculateHeightAndGradient(const std::vector<float>& nodes, rfm::Vector2 pos);
private:

	static void InitializeBrush(int radius);
	struct BrushIndicesAndWeights
	{
		bool initialized = false;
		std::vector<std::vector<int>> indices;
		std::vector<std::vector<float>> weights;
	};
	static BrushIndicesAndWeights s_brush;
};