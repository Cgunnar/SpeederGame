#pragma once
#include "GuiDebug.h"
class TerrainGUI : public GuiDebug
{
public:
	TerrainGUI(const std::string& name);
	virtual void Show() override;

	Slider slider1;
};

