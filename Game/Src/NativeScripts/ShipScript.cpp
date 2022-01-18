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
		m_controllInputPRY = Vector3(
			-gPad.thumbSticks.leftY * m_pitchSpeed,
			0,
			gPad.thumbSticks.leftX * m_rollSpeed);

		m_controllInputPRY = shipRotation * m_controllInputPRY;

		m_controllInputXYZ = Vector3(0, gPad.triggers.left * m_thrustSpeed, gPad.triggers.right * m_thrustSpeed);
		m_controllInputXYZ = shipRotation * m_controllInputXYZ;

		m_cameraPitch += gPad.thumbSticks.rightY * dt;
		m_cameraYaw += gPad.thumbSticks.rightX * dt;


		ImGui::Text("aoa: %f", rfm::RadToDeg(CalcAOA()));
	}
}


void ShipScript::OnFixedUpdate(float dt)
{
	if (!m_docked)
	{
		RigidBody& rigidBody = GetComponent<RigidBodyComp>()->rigidBody;
		rigidBody.velocity += m_controllInputXYZ * dt;
		rigidBody.angularVelocity += m_controllInputPRY * dt;
	}
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

float ShipScript::CalcAOA()
{
	if (m_docked) return 0;
	const RigidBody& rg = GetComponent<RigidBodyComp>()->rigidBody;
	const Transform& tr = GetComponent<TransformComp>()->transform;

	//velocity forward and up
	Vector3 fwUpVelocityDir = rfm::ProjectVectorOnPlane(rg.velocity, Plane(tr.right()));
	if (fwUpVelocityDir.length() == 0) return 0;
	fwUpVelocityDir.normalize();
	float angle = acos(dot(fwUpVelocityDir, tr.forward()));
	if(dot(fwUpVelocityDir, tr.up()) > 0) angle*=-1;
	return angle;
}
