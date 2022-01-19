#include "pch.hpp"
#include "ShipScript.h"
#include "Input.h"
#include "RenderComponents.h"
#include "PhysicsComponents.h"
#include "Geometry.h"
#include "AssetManager.h"
#include "RfextendedMath.hpp"
#include "imgui.h"

using namespace rfm;
using namespace rfe;

void ShipScript::OnStart()
{
	Material redWireFrame;
	redWireFrame.emissiveFactor = { 1,0,0 };
	redWireFrame.baseColorFactor = { 0,0,0,1 };
	redWireFrame.flags = RenderFlag::wireframe | RenderFlag::noBackFaceCull;
	AABB shipAABB = AssetManager::Get().GetModel(GetComponent<RenderModelComp>()->ModelID).aabb;
	Geometry::AABB_POS_NOR_UV shipBoundingBox(shipAABB);
	Mesh boxMesh = Mesh(shipBoundingBox.VertexData(), shipBoundingBox.IndexData(), shipAABB);
	AddComponent<RenderUnitComp>(boxMesh, redWireFrame);

	GetComponent<TransformComp>()->transform.setRotationDeg(30, 0, 10);
	Vector3 whd = shipAABB.GetWidthHeightDepth();
	m_rigidBodyDockCopy.mass = 2000;
	m_rigidBodyDockCopy.momentOfInertia[0][0] = (1.0f / 12.0f) * m_rigidBodyDockCopy.mass * (whd.y * whd.y + whd.z * whd.z);
	m_rigidBodyDockCopy.momentOfInertia[1][1] = (1.0f / 12.0f) * m_rigidBodyDockCopy.mass * (whd.x * whd.x + whd.z * whd.z);
	m_rigidBodyDockCopy.momentOfInertia[2][2] = (1.0f / 12.0f) * m_rigidBodyDockCopy.mass * (whd.x * whd.x + whd.y * whd.y);
	m_rigidBodyDockCopy.frictionCof = 0.7f;

	AddComponent<AABBComp>()->aabb = shipAABB;
}
constexpr float AirDensity = 1.225f;
void ShipScript::OnUpdate(float dt)
{
	auto gPad = Input::Get().GamePadState();
	auto gPadOld = Input::Get().OldGamePadState();
	if (gPad.IsConnected())
	{
		if (gPad.IsBPressed() && !gPadOld.IsBPressed())
		{
			reset();
		}
		if (gPad.IsAPressed() && !gPadOld.IsAPressed())
		{
			m_docked ? UnDockShip() : DockShip();
		}
		Transform tr = GetComponent<TransformComp>()->transform;
		
		auto [shipTranslation, shipRotation, shipScale] = decomposeToTRS(tr);
		m_controllInputPYR = Vector3(
			-gPad.thumbSticks.leftY * m_pitchSpeed,
			0,
			gPad.thumbSticks.leftX * m_rollSpeed);


		if (gPad.IsLeftShoulderPressed()) m_controllInputPYR.y += m_yawSpeed;
		if (gPad.IsRightShoulderPressed()) m_controllInputPYR.y -= m_yawSpeed;

		m_controllInputPYR = shipRotation * m_controllInputPYR;

		m_controllInputXYZ = Vector3(0, gPad.triggers.left * m_thrustSpeed, gPad.triggers.right * m_thrustSpeed);
		m_controllInputXYZ = shipRotation * m_controllInputXYZ;

		m_cameraPitch += gPad.thumbSticks.rightY * dt;
		m_cameraYaw += gPad.thumbSticks.rightX * dt;


		ImGui::Text("aoa: %f", rfm::RadToDeg(CalcAOA(GetRigidBody().velocity)));
		ImGui::Text("aos: %f", rfm::RadToDeg(CalcAOS(GetRigidBody().velocity)));




		//should be fixed

		if(!m_docked)
		{
			RigidBody& rigidBody = GetComponent<RigidBodyComp>()->rigidBody;
			Transform& transform = GetTransform();
			Matrix3 rot = transform.getRotationMatrix();
			Matrix3 rotT = transpose(rot);
			rigidBody.velocity += m_controllInputXYZ * dt;
			rigidBody.angularVelocity += m_controllInputPYR * dt;

			Vector3 airVelocity = rigidBody.velocity - Vector3(0, 0, 0); // - winds Velocity
			if (airVelocity.length() > 1)
			{
				float aoa = CalcAOA(airVelocity);

				float dynamicPressure = 0.5f * AirDensity * airVelocity.length() * airVelocity.length();

				float Cm = -0.0025f * aoa;

				Vector3 airVelDir = normalize(airVelocity);
				float area = abs(dot(transform.up(), airVelDir)) * m_topArea +
					abs(dot(transform.right(), airVelDir)) * m_sideArea +
					abs(dot(transform.forward(), airVelDir)) * m_frontArea;

				float drag = dynamicPressure * area * m_Cd;

				Vector3 dragForce = drag * -airVelDir;

				

				float pitchDampening = m_Cmq * m_chord * m_chord * m_liftSurfaceArea * dynamicPressure / (2 * airVelocity.length());
				float rollDampening = m_Clp * m_wingspann * m_wingspann * m_liftSurfaceArea * dynamicPressure / (2 * airVelocity.length());
				float yawDampening = m_Cnr * m_wingspann * m_wingspann * m_liftSurfaceArea * dynamicPressure / (2 * airVelocity.length());

				Vector3 angVelLocal = rotT * rigidBody.angularVelocity;

				Vector3 localMoment;
				localMoment.x = angVelLocal.x * pitchDampening;
				localMoment.y = angVelLocal.y * yawDampening;
				localMoment.z = angVelLocal.z * rollDampening;

				Vector3 localAngAcc = inverse(rigidBody.momentOfInertia) * localMoment;

				Vector3 angularAcc = rot * localAngAcc;
				rigidBody.angularVelocity += angularAcc * dt;
				rigidBody.velocity += (dragForce / rigidBody.mass) * dt;

				ImGui::Text("velocity: %s", rigidBody.velocity.ToString().c_str());
			}

		}


	}
}


void ShipScript::OnFixedUpdate(float dt)
{
	//if (!m_docked)
	//{
	//	RigidBody& rigidBody = GetComponent<RigidBodyComp>()->rigidBody;
	//	Transform& transform = GetTransform();
	//	rigidBody.velocity += m_controllInputXYZ * dt;
	//	rigidBody.angularVelocity += m_controllInputPYR * dt;

	//	Vector3 airVelocity = rigidBody.velocity - Vector3(0, 0, 0); // - winds Velocity
	//	if (airVelocity.length() > 0)
	//	{
	//		float aoa = CalcAOA(airVelocity);

	//		float dynamicPressure = 0.5f * AirDensity * airVelocity.length() * airVelocity.length();

	//		float Cm = -0.0025f * aoa;



	//		float pitchDampening = m_Cmq * m_chord * m_chord * m_surfaceArea * dynamicPressure / (2*airVelocity.length());
	//		float rollDampening = m_Clp * m_wingspann * m_wingspann * m_surfaceArea * dynamicPressure / (2 * airVelocity.length());
	//		float yawDampening = m_Cnr * m_wingspann * m_wingspann * m_surfaceArea * dynamicPressure / (2 * airVelocity.length());
	//		
	//		Vector3 angVelLocal = inverse((Matrix3)transform.getRotationMatrix()) * rigidBody.angularVelocity;
	//		if (isnan(angVelLocal.x))
	//		{
	//			std::cout << "nan\n";
	//		}

	//		ImGui::Text("localAngVel: %f %f %f", angVelLocal.x, angVelLocal.y, angVelLocal.z);
	//		Vector3 localMoment;
	//		localMoment.x = angVelLocal.x * pitchDampening;
	//		localMoment.y = angVelLocal.y * yawDampening;
	//		localMoment.z = angVelLocal.z * rollDampening;

	//		Vector3 globalMoment = (Matrix3)transform.getRotationMatrix() * localMoment;
	//		Vector3 angularAcc = rigidBody.momentOfInertia * globalMoment;
	//		//rigidBody.angularVelocity += angularAcc * dt;

	//	}

	//}
}



Matrix ShipScript::GetCameraFollowTransform()
{
	Transform tr = GetComponent<TransformComp>()->transform;
	tr.setRotation(0, 0, 0);
	m_followCamera = tr;
	m_followCamera.rotateL(-m_cameraPitch, m_cameraYaw, 0);
	m_followCamera.translateL(0, 0, -m_cameraArmLength);
	return m_followCamera;
}
void ShipScript::reset()
{
	Transform tr = GetComponent<TransformComp>()->transform;
	tr.setRotationDeg(30, 0, 10);
	tr.setTranslation(0, 5, 3);

	GetComponent<TransformComp>()->transform = tr;
	DockShip();
}

void ShipScript::DockShip()
{
	m_docked = true;
	auto rg = GetComponent<RigidBodyComp>();
	if (!rg) return;
	m_rigidBodyDockCopy = rg->rigidBody;
	m_rigidBodyDockCopy.angularVelocity = 0;
	m_rigidBodyDockCopy.velocity = 0;
	RemoveComponent<RigidBodyComp>();
}

void ShipScript::UnDockShip()
{
	m_docked = false;
	if (GetComponent<RigidBodyComp>()) return;
	AddComponent<RigidBodyComp>(m_rigidBodyDockCopy);
}

float ShipScript::CalcAOA(rfm::Vector3 airVelocity)
{
	if (m_docked) return 0;

	const Transform& tr = GetTransform();

	//velocity forward and up
	Vector3 fwUpVelocityDir = rfm::ProjectVectorOnPlane(airVelocity, Plane(tr.right()));
	if (fwUpVelocityDir.length() == 0) return 0;
	fwUpVelocityDir.normalize();
	float angle = acos(dot(fwUpVelocityDir, tr.forward()));
	if(dot(fwUpVelocityDir, tr.up()) > 0) angle*=-1;
	return angle;
}

float ShipScript::CalcAOS(rfm::Vector3 airVelocity)
{
	if (m_docked) return 0;

	const Transform& tr = GetTransform();

	//velocity forward and right
	Vector3 fwRgVelocityDir = rfm::ProjectVectorOnPlane(airVelocity, Plane(tr.up()));
	if (fwRgVelocityDir.length() == 0) return 0;
	fwRgVelocityDir.normalize();
	float angle = acos(dot(fwRgVelocityDir, tr.forward()));
	if (dot(fwRgVelocityDir, tr.right()) < 0) angle *= -1;
	return angle;
}

RigidBody& ShipScript::GetRigidBody()
{
	if (m_docked)
		return m_rigidBodyDockCopy;
	else
		return GetComponent<RigidBodyComp>()->rigidBody;
}

Transform& ShipScript::GetTransform()
{
	return GetComponent<TransformComp>()->transform;
}
