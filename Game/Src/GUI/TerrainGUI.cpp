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

	static float x = 0;
	ImGui::SliderFloat("Slider32", &x, 0, 1);

	static float v[5] = { 0.390f, 0.575f, 0.565f, 1.000f };
	ImGui::Bezier( "easeOutSine", v );       // draw
	float y = ImGui::BezierValue( x, v ); // x delta in [0..1] range

	

	ImGui::Text("out%f", y);

	float slider[3] = { 0,0,0 };




	slider1.GetValue(slider);
	if (ImGui::SliderFloat3("Slider1", (float*)&slider, slider1.minVal, slider1.maxVal))
	{
		slider1.SetValue(slider);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset1") || slider1.changed)
	{
		slider1.value = slider1.defaultValue;
		slider1.changed = false;
	}


	ImGui::End();
}
