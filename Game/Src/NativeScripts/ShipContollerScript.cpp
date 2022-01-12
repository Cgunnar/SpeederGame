#include "pch.hpp"
#include "ShipContollerScript.h"
#include "Input.h"
#include "RenderComponents.h"
#include "PhysicsComponents.h"
#include "Geometry.h"
#include "AssetManager.h"
#include "CollisionDetection.h"
#include <algorithm>
#include <NativeScriptCollection.h>

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
		/*if (abs(shipScale[1][1] - 1.0f) > 0.01)
		{
			std::cout << "scale: " << shipScale[0][0] << " " << shipScale[1][1] << shipScale[2][2] << std::endl;
		}
			std::cout << shipScale[0][0] << " " << shipScale[1][1] << shipScale[2][2] << std::endl;*/

		Vector3 deltaAngVel = Vector3(
			-gPad.thumbSticks.leftY * m_pitchSpeed,
			-gPad.thumbSticks.rightX * m_yawSpeed,
			gPad.thumbSticks.leftX * m_rollSpeed);

		deltaAngVel = shipRotation * deltaAngVel;

		rg.angularVelocity += deltaAngVel * dt;

		Vector3 deltaVelocity = Vector3(0, gPad.triggers.left * m_thrustSpeed, gPad.triggers.right * m_thrustSpeed);
		deltaVelocity = shipRotation * deltaVelocity;
		rg.velocity += deltaVelocity * dt;

		GetComponent<RigidBodyComp>()->rigidBody = rg;
	}
}
void ShipContollerScript::reset()
{
	Transform tr = GetComponent<TransformComp>()->transform;
	RigidBody rg = GetComponent<RigidBodyComp>()->rigidBody;

	rg.angularVelocity = Vector3(0, 0, 0);
	rg.velocity = { 0,0,0 };
	tr.setRotationDeg(30, 0, 10);
	tr.setTranslation(0, 5, 3);

	GetComponent<TransformComp>()->transform = tr;
	GetComponent<RigidBodyComp>()->rigidBody = rg;
}

void ShipScript::OnStart()
{
	Material redWireFrame;
	redWireFrame.emissiveFactor = { 1,0,0 };
	redWireFrame.baseColorFactor = { 0,0,0,1 };
	redWireFrame.flags = RenderFlag::wireframe | RenderFlag::noBackFaceCull; //lol the shadowmap dont check this flag
	AABB shipAABB = AssetManager::Get().GetModel(GetComponent<RenderModelComp>()->ModelID).aabb;
	Geometry::AABB_POS_NOR_UV shipBoundingBox(shipAABB);
	Mesh boxMesh = Mesh(shipBoundingBox.VertexData(), shipBoundingBox.IndexData(), shipAABB);
	AddComponent<RenderUnitComp>(boxMesh, redWireFrame);

	GetComponent<TransformComp>()->transform.setRotationDeg(30, 0, 10);
	Vector3 whd = shipAABB.GetWidthHeightDepth();
	RigidBody rg;
	rg.mass = 2000;
	rg.momentOfInertia[0][0] = (1.0f / 12.0f) * rg.mass * (whd.y * whd.y + whd.z * whd.z);
	rg.momentOfInertia[1][1] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.z * whd.z);
	rg.momentOfInertia[2][2] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.y * whd.y);

	AddComponent<RigidBodyComp>()->rigidBody = rg;
	AddComponent<AABBComp>()->aabb = shipAABB;
}

void ShipScript::OnUpdate(float dt)
{
	Vector3 p = GetComponent<TransformComp>()->transform.getTranslation();
	//float h = EntityReg::GetComponentArray<TerrainScript>().front().HeightOverTerrain(p);

}
struct sumP
{
	float n = 0;
	float t = 0;
	float b = 0;
};

void ShipScript::OnFixedUpdate(float dt)
{
	Plane plane0 = Plane({ 0,1,0 }, 0);
	AABB aabb = GetComponent<AABBComp>()->aabb;
	Transform transform = GetComponent<TransformComp>()->transform;
	RigidBody rigidBody = GetComponent<RigidBodyComp>()->rigidBody;

	auto aabbPoints = aabb.GetPointsTransformed(transform);
	auto pointsUnderPlane = colDetect::PlaneVSPoints(plane0, { aabbPoints.begin(), aabbPoints.end() });
	std::vector<sumP> sumP;
	sumP.resize(pointsUnderPlane.size());
	const int kMax = 20;
	Matrix3 invI = inverse(rigidBody.momentOfInertia);
	float invMass = 1.0f / rigidBody.mass;
	rigidBody.velocity.y -= 9.82 * dt;
	Vector3 v = rigidBody.velocity;
	Vector3 w = rigidBody.angularVelocity;
	float biasFactor = 0.06f / (dt / static_cast<float>(kMax));
	for (int k = 0; k < kMax; k++)
	{
		for (int i = 0; i < pointsUnderPlane.size(); i++)
		{


			auto& p = pointsUnderPlane[i];
			Vector3 normal = p.normal;
			Vector3 tangent = Vector3(0,0,1);
			Vector3 biTangent = Vector3(1,0,0);
			Vector3 r = p.pointRealPosition - transform.getTranslation();
			Vector3 constaint = rigidBody.velocity + cross(r, rigidBody.angularVelocity);
			float constraintN = dot(constaint, normal);
			float constraintT = dot(constaint, tangent);
			float constraintB = dot(constaint, biTangent);
			float eMassN = invMass + dot(normal, cross(r, (invI * cross(normal, r))));
			float eMassT = invMass + dot(tangent, cross(r, (invI * cross(tangent, r))));
			float eMassB = invMass + dot(biTangent, cross(r, (invI * cross(biTangent, r))));
			float b = biasFactor * std::max(p.penetration - 0.01f, 0.0f);
			float pCorrectedN = (-constraintN + b) / eMassN;
			float pCorrectedT = (-constraintT) / eMassT;
			float pCorrectedB = (-constraintB) / eMassB;

			float oldP = sumP[i].n;
			sumP[i].n = std::max(sumP[i].n + pCorrectedN, 0.0f);
			pCorrectedN = sumP[i].n - oldP;
			rigidBody.velocity += invMass * pCorrectedN * normal;
			rigidBody.angularVelocity += invI * pCorrectedN * cross(normal, r);

			oldP = sumP[i].t;
			sumP[i].t = std::clamp(sumP[i].t + pCorrectedT, -m_friction * sumP[i].n, m_friction * sumP[i].n);
			pCorrectedT = sumP[i].t - oldP;
			rigidBody.velocity += invMass * pCorrectedT * tangent;
			rigidBody.angularVelocity += invI * pCorrectedT * cross(tangent, r);

			oldP = sumP[i].b;
			sumP[i].b = std::clamp(sumP[i].b + pCorrectedB, -m_friction * sumP[i].n, m_friction * sumP[i].n);
			pCorrectedB = sumP[i].b - oldP;
			rigidBody.velocity += invMass * pCorrectedB * biTangent;
			rigidBody.angularVelocity += invI * pCorrectedB * cross(biTangent, r);
		}
	}

	transform.translateW(rigidBody.velocity * dt); //update the position
	if (rigidBody.angularVelocity.length() > 0) //update the rotation
		transform.rotateW(rotationMatrixFromNormal(
			normalize(rigidBody.angularVelocity), -rigidBody.angularVelocity.length() * dt));

	GetComponent<RigidBodyComp>()->rigidBody = rigidBody;
	GetComponent<TransformComp>()->transform = transform;
}
