#pragma once
#include <string>
#include "RimfrostMath.hpp"

class GuiTest;

class Slider
{
	friend GuiTest;
public:
	rfm::Vector3 value;
	void ChangeDefaultValues(rfm::Vector3 val, float min = -10, float max = 10)
	{
		defaultValue = val;
		minVal = min; maxVal = max;
		changed = true;
	}
private:
	rfm::Vector3 defaultValue;
	float minVal = -1;
	float maxVal = 1;
	bool changed = true;

	void GetValue(float arr[3]) 
	{
		arr[0] = value.x; arr[1] = value.y; arr[2] = value.z;
	}
	void SetValue(float arr[3])
	{
		value = rfm::Vector3(arr[0], arr[1], arr[2]);
	}
};

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

	Slider slider1;
	Slider slider2;

private:
};

