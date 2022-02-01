#include "pch.hpp"
#include "ShipScript.h"
#include "Input.h"
#include "RenderComponents.h"
#include "PhysicsComponents.h"
#include "Geometry.h"
#include "AssetManager.h"
#include "RfextendedMath.hpp"
#include "imgui.h"
#include "TerrainScript.h"
#include "FrameTimer.hpp"

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



	//create bullet for main weapon
	float r = 0.05f;
	constexpr float mass = 2;
	m_mainWeaponProjectile.rigidBody.momentOfInertia = (2.0f / 5.0f) * r * r * mass * Matrix3();
	m_mainWeaponProjectile.rigidBody.mass = mass;
	m_mainWeaponProjectile.rigidBody.frictionCof = 0.1f;

	Material bulletMaterial;
	bulletMaterial.name = "bullet";
	bulletMaterial.baseColorFactor = { 0,0,0,1 };
	bulletMaterial.emissiveFactor = { 1,1,0 };
	Geometry::Sphere_POS_NOR_UV_TAN_BITAN sphere = Geometry::Sphere_POS_NOR_UV_TAN_BITAN(16, r);
	m_mainWeaponProjectile.aabb = AABB(-Vector3(r, r, r), Vector3(r, r, r));
	Mesh sphereMesh = Mesh(sphere.VertexData(), sphere.IndexData(), m_mainWeaponProjectile.aabb);
	m_mainWeaponProjectile.renderUnitID = AssetManager::Get().AddRenderUnit(sphereMesh, bulletMaterial);
}
constexpr float AirDensity = 1.225f;
void ShipScript::OnUpdate(float dt)
{
	m_controllInputPYR = Vector3(
		shipController.pitchInput * m_pitchSpeed,
		shipController.yawInput * m_yawSpeed,
		shipController.rollInput * m_rollSpeed);
	Matrix3 shipRotation = GetComponent<TransformComp>()->transform.getRotationMatrix();
	m_controllInputPYR = shipRotation * m_controllInputPYR;

	m_controllInputXYZ = Vector3(0, shipController.secondaryThrottle * m_thrustSpeed, shipController.mainThrottle * m_thrustSpeed);
	m_controllInputXYZ = shipRotation * m_controllInputXYZ;

	if (shipController.dockToggle)
		m_docked ? UnDockShip() : DockShip();
	if (shipController.reset)
		reset();

	m_cameraPitch = shipController.cameraPitch;
	m_cameraYaw = shipController.cameraYaw;

	if (shipController.fireMainWeapon)
		FireMainWeapon();

	ImGui::Text("aoa: %f", rfm::RadToDeg(CalcAOA(GetRigidBody().velocity)));
	ImGui::Text("aos: %f", rfm::RadToDeg(CalcAOS(GetRigidBody().velocity)));


	ImGui::Text("velocity: %s", GetRigidBody().velocity.ToString().c_str());


	auto& t = EntityReg::GetComponentArray<TerrainScript>().front();
	float altitude = t.GetHeightOverTerrain(GetTransform().getTranslation());
	ImGui::Text("altitude: %f", altitude);
	ImGui::Text("resting: %s", std::to_string(GetRigidBody().resting).c_str());


	//remove old bullets
	while (!m_mainWeaponBullets.empty() && FrameTimer::TimeFromLaunch() - m_mainWeaponBullets.front().spawnTime > m_mainWeaponProjectile.lifeTime)
		m_mainWeaponBullets.pop();
}


void ShipScript::OnFixedUpdate(float dt)
{
	if (!m_docked)
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


		}

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

void ShipScript::FireMainWeapon()
{
	static double lastTime = 0;
	if (FrameTimer::TimeFromLaunch() - lastTime > 1.0 / m_mainWeaponProjectile.roundsPerSecond)
	{
		lastTime = FrameTimer::TimeFromLaunch();
		Entity projectile = EntityReg::CreateEntity();
		projectile.AddComponent<TransformComp>()->transform = GetTransform();
		auto projRg = projectile.AddComponent<RigidBodyComp>();
		projRg->rigidBody = m_mainWeaponProjectile.rigidBody;
		projRg->rigidBody.velocity = GetRigidBody().velocity + m_mainWeaponProjectile.speed * GetTransform().forward();

		auto rendUnit = projectile.AddComponent<RenderUnitComp>()->unitID = m_mainWeaponProjectile.renderUnitID;
		projectile.AddComponent<AABBComp>()->aabb = m_mainWeaponProjectile.aabb;
		m_mainWeaponBullets.emplace(projectile, FrameTimer::TimeFromLaunch());
	}
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
	if (dot(fwUpVelocityDir, tr.up()) > 0) angle *= -1;
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
