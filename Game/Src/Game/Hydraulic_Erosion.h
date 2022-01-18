#pragma once
#include "TerreinTypes.h"


class ErosionSimulator
{
public:
	static void Init(int radius);
	static void Erode(TerrainMap& map);
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