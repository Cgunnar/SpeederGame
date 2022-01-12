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

	rg.angularVelocity = Vector3(0, 0, 0);
	rg.velocity = { 0,0,0 };
	tr.setRotationDeg(30, 0, 0);
	tr.setTranslation(0, 10, 3);

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
	GetComponent<TransformComp>()->transform.setRotationDeg(30, 0, 0);

	Vector3 whd = shipAABB.GetWidthHeightDepth();
	RigidBody rg;
	//rg.mass = 0.1f;
	rg.mass = 10.0f;
	rg.momentOfInertia[0][0] = (1.0f / 12.0f) * rg.mass * (whd.y * whd.y + whd.z * whd.z);
	rg.momentOfInertia[1][1] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.z * whd.z);
	rg.momentOfInertia[2][2] = (1.0f / 12.0f) * rg.mass * (whd.x * whd.x + whd.y * whd.y);

	AddComponent<RigidBodyComp>()->rigidBody = rg;
	AddComponent<AABBComp>()->aabb = shipAABB;
}

void ShipScript::OnUpdate(float dt)
{
	Plane plane0 = Plane({ 0,1,0 }, 0);
	AABB aabb = GetComponent<AABBComp>()->aabb;
	Transform transform = GetComponent<TransformComp>()->transform;
	RigidBody rigidBody = GetComponent<RigidBodyComp>()->rigidBody;

	auto aabbPoints = aabb.GetPointsTransformed(transform);
	auto pointsUnderPlane = colDetect::PlaneVSPoints(plane0, {aabbPoints.begin(), aabbPoints.end()});
	std::vector<float> sumP;
	sumP.resize(pointsUnderPlane.size(), 0);
	const int kMax = 20;

	Matrix3 invI = inverse(rigidBody.momentOfInertia);
	float invMass = 1.0f / rigidBody.mass;
	rigidBody.velocity.y -= 9.82 * dt;
	for (int k = 0; k < kMax; k++)
	{
		for (int i = 0; i < pointsUnderPlane.size(); i++)
		{
			auto& p = pointsUnderPlane[i];
			Vector3 normal = p.normal;
			Vector3 r = p.pointRealPosition - transform.getTranslation();

			Vector3 invIcrossNormR = invI * cross(normal, r);
			float constraint = dot(rigidBody.velocity + cross(r, rigidBody.angularVelocity), normal);
			float eMass = invMass + dot(invIcrossNormR, invIcrossNormR);
			float pCorrected = -constraint / eMass;

			float oldP = sumP[i];
			sumP[i] += pCorrected;
			sumP[i] = std::max(sumP[i], 0.0f);
			float diffP = sumP[i] - oldP;

			rigidBody.velocity += invMass * diffP * normal;
			rigidBody.angularVelocity += invI * diffP * cross(normal, r);
		}
	}

	transform.translateW(rigidBody.velocity * dt); //update the position
	if (rigidBody.angularVelocity.length() > 0) //update the rotation
		transform.rotateW(rotationMatrixFromNormal(
			normalize(rigidBody.angularVelocity), -rigidBody.angularVelocity.length() * dt));

	GetComponent<RigidBodyComp>()->rigidBody = rigidBody;
	GetComponent<TransformComp>()->transform = transform;
}
