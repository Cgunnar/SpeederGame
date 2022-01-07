#include "pch.hpp"
#include "TerrainGUI.h"
#include <imgui.h>
#include "Curve_editor.h"

TerrainGUI::TerrainGUI(const std::string& name) : GuiDebug(name)
{

}

void TerrainGUI::Show()
{
	ImGui::Begin(m_name.c_str());
	
	ImGui::SliderFloat("persistence", &m_values.persistence, 0, 1);
	ImGui::SliderFloat("lacunarity", &m_values.lacunarity, 0, 3);
	ImGui::SliderFloat("frequencyScale", &m_values.frequencyScale, 0.1f, 100);
	ImGui::SliderFloat("heightScale", &m_values.heightScale, 1, 100);
	ImGui::SliderFloat2("baseOffset", (float*)&m_values.baseOffset, -100, 100);
	ImGui::SliderInt("octaves", &m_values.octaves, 1, 10);
	static int seed = m_values.seed;
	ImGui::SliderInt("seed", &seed, 0, 10);
	m_values.seed = seed;


	ImGui::End();
}

TerrainGUIValues TerrainGUI::GetValues() const
{
	return m_values;
}
