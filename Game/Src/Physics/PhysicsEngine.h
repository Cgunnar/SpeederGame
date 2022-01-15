#pragma once

class PhysicsEngine
{
public:
	PhysicsEngine(double timeStep = 1.0 / 100.0, float g = 9.82);
	void Run(double dt);

private:
	void ApplyGravity();

	double m_timeStep;
	float m_g;
};

