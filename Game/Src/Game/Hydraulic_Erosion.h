#pragma once
#include "TerreinTypes.h"


class ErosionSimulator
{
public:

	static void InitializeBrush(int mapSize, int radius);
	static void Erode(TerrainMap& map);
private:

	struct BrushIndicesAndWeights
	{
		bool initialized = false;
		int size = 0;
		std::vector<std::vector<int>> indices;
		std::vector<std::vector<float>> weights;
	};
	static BrushIndicesAndWeights s_brush;
};