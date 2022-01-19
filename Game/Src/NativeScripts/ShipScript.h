#pragma once
#include "NativeScript.h"
#include "RimfrostMath.hpp"
#include "PhysicsComponents.h"

class ShipScript : public rfe::NativeScriptComponent<ShipScript>
{
public:
	void OnStart();
	void OnUpdate(float dt);
	void OnFixedUpdate(float dt);
	rfm::Matrix GetCameraFollowTransform();
private:
	void reset();
	void DockShip();
	void UnDockShip();
	float CalcAOA(rfm::Vector3 airVelocity);
	float CalcAOS(rfm::Vector3 airVelocity);
	RigidBody& GetRigidBody();
	rfm::Transform& GetTransform();

	bool m_docked = true;
	float m_yawSpeed = rfm::DegToRad(20);
	float m_pitchSpeed = rfm::DegToRad(80);
	float m_rollSpeed = rfm::DegToRad(50);
	float m_thrustSpeed = 14;

	rfm::Vector3 m_controllInputPYR;
	rfm::Vector3 m_controllInputXYZ;

	rfm::Transform m_followCamera;
	RigidBody m_rigidBodyDockCopy;
	float m_cameraYaw = 0;
	float m_cameraPitch = 0;
	float m_cameraArmLength = 4;
	float m_friction = 0.7f;

	//airplane stuff
	float m_Cmq = -0.7; //pitchdampeningCof
	float m_Clp = -0.1f; //rolldampeningCof
	float m_Cnr = -0.2f; //yawdampeningCof
	float m_chord = 5;
	float m_wingspann = 8;
	float m_surfaceArea = 20;
};
