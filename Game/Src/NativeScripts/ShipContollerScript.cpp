#include "pch.hpp"
#include "ShipContollerScript.h"
#include "Input.h"
#include "RenderComponents.h"
#include "PhysicsComponents.h"
#include "Geometry.h"
#include "AssetManager.h"
#include "CollisionDetection.h"

using namespace rfm;
using namespace rfe;

void ShipContollerScript::OnUpdate(float dt)
{
	auto gPad = Input::Get().GamePad().GetState(0);

	if (gPad.IsConnected())
	{
		
		auto& transform = GetComponent<TransformComp>()->transform;
		transform.rotateDegL(
			dt * gPad.thumbSticks.leftY * m_pitchSpeed,
			dt * gPad.thumbSticks.rightX * m_yawSpeed,
			dt * -gPad.thumbSticks.leftX * m_rollSpeed);

		transform.translateL(0, 0, m_thrustSpeed * dt * gPad.triggers.right);

		if (gPad.IsAPressed()) Input::Get().GamePad().SetVibration(0, 1, 1);
		else Input::Get().GamePad().SetVibration(0, 0, 0);
	}
}

void ShipScript::OnStart()
{
	Material redWireFrame;
	redWireFrame.emissiveFactor = { 1,0,0 };
	redWireFrame.baseColorFactor = 0;
	redWireFrame.flags = RenderFlag::wireframe | RenderFlag::noBackFaceCull;
	GID shipModelID = GetComponent<RenderModelComp>()->ModelID;
	AABB shipAABB = AssetManager::Get().GetModel(shipModelID).aabb;
	Geometry::AABB_POS_NOR_UV shipBoundingBox(shipAABB);
	Mesh boxMesh = Mesh(shipBoundingBox.VertexData(), shipBoundingBox.IndexData(), shipAABB);
	AddComponent<RenderUnitComp>(boxMesh, redWireFrame);

	GetComponent<TransformComp>()->transform.setRotationDeg(30, 0, 10);

	Vector3 whd = shipAABB.GetWidthHeightDepth();
	RigidBody rg;
	rg.mass = 10;
	rg.momentOfInertia[0][0] = (1.0f / 12.0f) * rg.mass * (whd.y * whd.y + whd.z * whd.z);
	rg.momentOfInertia[1][1] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.z * whd.z);
	rg.momentOfInertia[2][2] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.y * whd.y);

	AddComponent<RigidBodyComp>()->rigidBody = rg;
	AddComponent<AABBComp>()->aabb = shipAABB;

}

void ShipScript::OnUpdate(float dt)
{
	//float g = 9.82f;
	float g = 0.5f;
	//fix real ecs systems later

	Plane plane0 = Plane({ 0,1,0 }, 0);
	Transform tr = GetComponent<TransformComp>()->transform;
	AABB aabb = GetComponent<AABBComp>()->aabb;
	RigidBody rg = GetComponent<RigidBodyComp>()->rigidBody;
	Vector3 pos = tr.getTranslation();
	

	rg.velocity.y -= g*dt;

	auto aabbPoints = aabb.GetPointsTransformed(tr);
	auto collPoints = colDetect::PlaneVSPoints(plane0, {aabbPoints.begin(), aabbPoints.end()});

	if (!collPoints.empty())
	{
		float pen = collPoints.front().penetration;
		tr.translateW(pen * collPoints.front().normal);
	}

	for (auto& p : collPoints)
	{
		Vector3 collPoint = p.intersectionPoint;
		Vector3 normal = p.normal;
		Vector3 r = collPoint - tr.getTranslation();
		float C = dot(rg.velocity + cross(r, rg.angularVelocity), normal);
		float invMass = 1.0f / rg.mass;
		float invI = 1; // fix

		float eMass = 1.0f / (invMass + invI * dot(cross(normal, r), cross(normal, r)));
		float lambda = -eMass * C;
		rg.velocity = rg.velocity + invMass * lambda * normal;
		rg.angularVelocity = rg.angularVelocity + invI * lambda * cross(normal, r);




		//break;
	}


	tr.translateW(rg.velocity * dt);
	if (rg.angularVelocity.length() > 0)
	{
		tr.rotateW(rotationMatrixFromNormal(normalize(rg.angularVelocity), -rg.angularVelocity.length() * dt));
	}

	GetComponent<RigidBodyComp>()->rigidBody = rg;
	GetComponent<TransformComp>()->transform = tr;
}
