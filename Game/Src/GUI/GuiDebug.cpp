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

	
	
	
	slider[0] = Slider1.x; slider[1] = Slider1.y; slider[2] = Slider1.z;
	if (ImGui::SliderFloat3("Slider1", (float*)&slider, -5, 5))
	{
		Slider1 = Vector3(slider[0], slider[1], slider[2]);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset1"))
	{
		Slider1 = Vector3(0, 0, 0);
	}
	
	
	
	
	slider[0] = Slider2.x; slider[1] = Slider2.y; slider[2] = Slider2.z;
	if (ImGui::SliderFloat3("Slider2", (float*)&slider, -5, 5))
	{
		Slider2 = Vector3(slider[0], slider[1], slider[2]);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset2"))
	{
		Slider2 = Vector3(0, 0, 0);
	}
	


	ImGui::End();
}
