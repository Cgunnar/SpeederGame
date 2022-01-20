#include "pch.hpp"
#include "TerrainGUI.h"
#include <imgui.h>
#include "Curve_editor.h"

TerrainGUI::TerrainGUI(const std::string& name) : GuiDebug(name)
{
	m_valuesDefault.octaves = 10;
	m_valuesDefault.scale = 1;
	m_valuesDefault.frequencyScale = 150;
	m_valuesDefault.lacunarity = 2;
	m_valuesDefault.heightScale = 100;
	m_valuesDefault.seed = 10;
	m_values = m_valuesDefault;
}

bool TerrainGUI::Show()
{
	bool c = false;
	ImGui::Begin(m_name.c_str());
	
	c = c || ImGui::SliderFloat("scale", &m_values.scale, 0.01, 2);
	c = c || ImGui::SliderFloat("persistence", &m_values.persistence, 0, 1);
	c = c || ImGui::SliderFloat("lacunarity", &m_values.lacunarity, 0, 2);
	c = c || ImGui::SliderFloat("frequencyScale", &m_values.frequencyScale, 1, 6000);
	c = c || ImGui::SliderFloat("heightScale", &m_values.heightScale, 1, 500);
	c = c || ImGui::SliderFloat2("baseOffset", (float*)&m_values.baseOffset, -50, 50);
	c = c || ImGui::SliderInt("octaves", &m_values.octaves, 1, 20);
	c = c || ImGui::SliderInt("erosion", &m_values.erosionIterations, 0, 100000);
	static int seed = m_values.seed;
	c = c || ImGui::SliderInt("seed", &seed, 0, 10);
	m_values.seed = seed;
	if (ImGui::Button("reset"))
	{
		m_values = m_valuesDefault;
		c = true;
	}


	ImGui::End();
	return c;
}

TerrainGUIValues TerrainGUI::GetValues() const
{
	return m_values;
}
