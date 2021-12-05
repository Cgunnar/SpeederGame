#include "pch.hpp"
#include "GuiDebug.h"
#include <imgui.h>

using namespace rfm;

GuiDebug::GuiDebug(const std::string& name) : m_name(name)
{
}

GuiTest::GuiTest(const std::string& name) : GuiDebug(name)
{
}

void GuiTest::Show()
{
	ImGui::Begin(m_name.c_str());

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
	
	
	
	
	slider2.GetValue(slider);
	if (ImGui::SliderFloat3("Slider2", (float*)&slider, slider2.minVal, slider2.maxVal))
	{
		slider2.SetValue(slider);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset2") || slider2.changed)
	{
		slider2.value = slider2.defaultValue;
		slider2.changed = false;
	}
	


	ImGui::End();
}
