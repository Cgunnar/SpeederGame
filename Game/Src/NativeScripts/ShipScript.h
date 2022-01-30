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

	struct ShipControls
	{
		//use this struct for now to remove xbox controller input from ShipScript
		float mainThrottle = 0;
		float secondaryThrottle = 0;
		float pitchInput = 0;
		float yawInput = 0;
		float rollInput = 0;
		bool dockToggle = true;
		bool fireMainWeapon = false;
		bool reset = false;
		float cameraYaw = 0;
		float cameraPitch = 0;
	} shipController;

private:
	void reset();
	void DockShip();
	void UnDockShip();
	float CalcAOA(rfm::Vector3 airVelocity);
	float CalcAOS(rfm::Vector3 airVelocity);
	RigidBody& GetRigidBody();
	rfm::Transform& GetTransform();

	bool m_docked = true;
	float m_yawSpeed = rfm::DegToRad(30);
	float m_pitchSpeed = rfm::DegToRad(100);
	float m_rollSpeed = rfm::DegToRad(70);
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
	float m_Cd = 0.25f; //drag cof
	float m_Cmq = -0.2f;//= -0.7f; //pitchdampeningCof
	float m_Clp = -0.04f;//= -0.1f; //rolldampeningCof
	float m_Cnr = -0.07; //= -0.2f; //yawdampeningCof
	float m_chord = 5;
	float m_wingspann = 8;
	float m_liftSurfaceArea = 20;

	float m_frontArea = 4;
	float m_topArea = 32;
	float m_sideArea = 12;
};
