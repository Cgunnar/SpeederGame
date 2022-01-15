#include "pch.hpp"
#include "PhysicsEngine.h"
#include "FrameTimer.hpp"
#include "TerrainVsBoxPhysics.h"
#include "rfEntity.hpp"
#include "PhysicsComponents.h"

using namespace rfe;
using namespace rfm;

PhysicsEngine::PhysicsEngine(double timeStep, float g) : m_timeStep(timeStep), m_g(g)
{

}

void PhysicsEngine::Run(double dt)
{
	static double deltaTime = 0.0;
	deltaTime += dt;
	while (m_timeStep < deltaTime)
	{
		ApplyGravity();
		phySys::FindAndResolveTerrainBoxCollision(m_timeStep);
		deltaTime -= m_timeStep;
	}
}

void PhysicsEngine::ApplyGravity()
{
	auto& rigidBodys = EntityReg::GetComponentArray<RigidBodyComp>();
	for (auto& r : rigidBodys)
	{
		r.rigidBody.velocity.y -= m_g * m_timeStep;
	}
}
