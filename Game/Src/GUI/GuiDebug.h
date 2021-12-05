#pragma once
#include <string>
#include "RimfrostMath.hpp"


class GuiDebug
{
public:
	GuiDebug(const std::string& name);
	virtual void Show() = 0;

protected:
	std::string m_name;
};

class GuiTest : public GuiDebug
{
public:
	GuiTest(const std::string& name);
	virtual void Show() override;

	rfm::Vector3 Slider1;
	rfm::Vector3 Slider2;

private:
};

