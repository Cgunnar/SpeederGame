#include "pch.hpp"
#include "TerrainGUI.h"
#include <imgui.h>
#include "Curve_editor.h"

TerrainGUI::TerrainGUI(const std::string& name) : GuiDebug(name)
{
	m_values.octaves = 8;
	m_values.frequencyScale = 50;
	m_values.lacunarity = 2;
	m_values.heightScale = 50;
	m_values.seed = 32;
}

bool TerrainGUI::Show()
{
	bool c = false;
	ImGui::Begin(m_name.c_str());
	
	c = c || ImGui::SliderFloat("scale", &m_values.scale, 0.01, 1);
	c = c || ImGui::SliderFloat("persistence", &m_values.persistence, 0, 1);
	c = c || ImGui::SliderFloat("lacunarity", &m_values.lacunarity, 0, 3);
	c = c || ImGui::SliderFloat("frequencyScale", &m_values.frequencyScale, 0.1f, 100);
	c = c || ImGui::SliderFloat("heightScale", &m_values.heightScale, 1, 100);
	c = c || ImGui::SliderFloat2("baseOffset", (float*)&m_values.baseOffset, -50, 50);
	c = c || ImGui::SliderInt("octaves", &m_values.octaves, 1, 10);
	static int seed = m_values.seed;
	c = c || ImGui::SliderInt("seed", &seed, 0, 10);
	m_values.seed = seed;


	ImGui::End();
	return c;
}

TerrainGUIValues TerrainGUI::GetValues() const
{
	return m_values;
}
