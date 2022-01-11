#include "pch.hpp"
#include "ShipContollerScript.h"
#include "Input.h"
#include "RenderComponents.h"
#include "PhysicsComponents.h"
#include "Geometry.h"
#include "AssetManager.h"
#include "CollisionDetection.h"
#include <algorithm>

using namespace rfm;
using namespace rfe;

void ShipContollerScript::OnUpdate(float dt)
{
	auto gPad = Input::Get().GamePad().GetState(0);

	if (gPad.IsConnected())
	{
		if (gPad.IsBPressed())
		{
			reset();
		}
		Transform tr = GetComponent<TransformComp>()->transform;
		RigidBody rg = GetComponent<RigidBodyComp>()->rigidBody;

		auto [shipTranslation, shipRotation, shipScale] = decomposeToTRS(tr);

		Vector3 deltaAngVel = Vector3(
			-gPad.thumbSticks.leftY * m_pitchSpeed,
			-gPad.thumbSticks.rightX * m_yawSpeed,
			gPad.thumbSticks.leftX * m_rollSpeed);

		deltaAngVel = shipRotation * deltaAngVel;

		rg.angularVelocity += deltaAngVel * dt;

		Vector3 deltaVelocity = Vector3(0, 3 * gPad.triggers.left * m_thrustSpeed, gPad.triggers.right * m_thrustSpeed);
		deltaVelocity = shipRotation * deltaVelocity;
		rg.velocity += deltaVelocity * dt;

		GetComponent<RigidBodyComp>()->rigidBody = rg;
	}
}

void ShipContollerScript::reset()
{
	Transform tr = GetComponent<TransformComp>()->transform;
	RigidBody rg = GetComponent<RigidBodyComp>()->rigidBody;

	rg.angularVelocity = { 0,0,0 };
	rg.velocity = { 0,0,0 };
	tr.setRotation(0, 0, 0);
	tr.setTranslation(0, 10, 0);


	GetComponent<TransformComp>()->transform = tr;
	GetComponent<RigidBodyComp>()->rigidBody = rg;
}

void ShipScript::OnStart()
{
	Material redWireFrame;
	redWireFrame.emissiveFactor = { 0,0,0 };
	redWireFrame.baseColorFactor = { 1,0,0,1 };
	redWireFrame.flags = RenderFlag::wireframe | RenderFlag::noBackFaceCull;
	GID shipModelID = GetComponent<RenderModelComp>()->ModelID;
	AABB shipAABB = AssetManager::Get().GetModel(shipModelID).aabb;
	Geometry::AABB_POS_NOR_UV shipBoundingBox(shipAABB);
	Mesh boxMesh = Mesh(shipBoundingBox.VertexData(), shipBoundingBox.IndexData(), shipAABB);
	AddComponent<RenderUnitComp>(boxMesh, redWireFrame);

	GetComponent<TransformComp>()->transform.setRotationDeg(30, 0, 10);

	Vector3 whd = shipAABB.GetWidthHeightDepth();
	RigidBody rg;
	rg.mass = 0.1f;
	rg.momentOfInertia[0][0] = (1.0f / 12.0f) * rg.mass * (whd.y * whd.y + whd.z * whd.z);
	rg.momentOfInertia[1][1] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.z * whd.z);
	rg.momentOfInertia[2][2] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.y * whd.y);

	AddComponent<RigidBodyComp>()->rigidBody = rg;
	AddComponent<AABBComp>()->aabb = shipAABB;

}

void ShipScript::OnUpdate(float dt)
{
	//float g = 9.82f;
	float g = 3.0f;
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
	float grounded = !collPoints.empty();
	for (auto& p : collPoints)
	{
		//this runs for all collision points, this is naive and will lead to jitter
		//but i dont know how to do it for real...
		float my = 0.5f;
		Vector3 collPoint = p.intersectionPoint;
		Vector3 tangent = Vector3(0, 0, 1); // dont have this harcoded!
		Vector3 biTangent = Vector3(-1, 0, 0); // dont have this harcoded!
		Vector3 normal = p.normal;
		Vector3 r = collPoint - tr.getTranslation();


		Matrix3 invI = inverse(rg.momentOfInertia);
		float invMass = 1.0f / rg.mass;
		Vector3 physicsyStuffN = invI * cross(normal, r);
		Vector3 physicsyStuffT = invI * cross(tangent, r);
		Vector3 physicsyStuffB = invI * cross(biTangent, r);
		float eMassN = invMass + dot(physicsyStuffN, physicsyStuffN);
		float eMassT = invMass + dot(physicsyStuffT, physicsyStuffT);
		float eMassB = invMass + dot(physicsyStuffB, physicsyStuffB);
		float constraintN = dot(rg.velocity + cross(r, rg.angularVelocity), normal);
		float constraintT = dot(rg.velocity + cross(r, rg.angularVelocity), tangent);
		float constraintB = dot(rg.velocity + cross(r, rg.angularVelocity), biTangent);

		float lambdaN = std::max(-constraintN / eMassN, 0.0f);
		float lambdaT = std::clamp( -constraintT/ eMassT, -my * lambdaN, my*lambdaN);
		float lambdaB = std::clamp( -constraintB/ eMassT, -my * lambdaN, my*lambdaN);
		
		rg.velocity = rg.velocity + invMass * lambdaN * normal;
		rg.angularVelocity = rg.angularVelocity + invI * lambdaN * cross(normal, r);
		rg.velocity = rg.velocity + invMass * lambdaT * tangent;
		rg.angularVelocity = rg.angularVelocity + invI * lambdaT * cross(tangent, r);
		rg.velocity = rg.velocity + invMass * lambdaB * biTangent;
		rg.angularVelocity = rg.angularVelocity + invI * lambdaB * cross(biTangent, r);

	}

	tr.translateW(rg.velocity * dt);
	if (rg.angularVelocity.length() > 0)
	{
		tr.rotateW(rotationMatrixFromNormal(normalize(rg.angularVelocity), -rg.angularVelocity.length() * dt));
	}

	GetComponent<RigidBodyComp>()->rigidBody = rg;
	GetComponent<TransformComp>()->transform = tr;
}
