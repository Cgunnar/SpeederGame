#pragma once
#include "GuiDebug.h"
#include "TerreinTypes.h"
struct TerrainGUIValues
{
	float scale = 1;
	float frequencyScale = 10;
	float heightScale = 10;
	int octaves = 1;
	float persistence = 0.5f;
	float lacunarity = 1;
	int erosionIterations = 20000;
	rfm::Vector2 baseOffset;
	uint32_t seed = 123456u;
	rfm::Vector2 uvScale = { 0,0 }; //set to 0,0 to use width, height
	/*std::vector<Biom> bioms;
	std::vector<LODinfo> LODs;
	std::function<float(float)> heightScaleFunc = [](float s) { return s; };*/
};

class TerrainGUI : public GuiDebug
{
	TerrainGUIValues m_values;
public:
	TerrainGUIValues m_valuesDefault;
	TerrainGUI(const std::string& name);
	virtual bool Show() override;
	TerrainGUIValues GetValues() const;
	
};

