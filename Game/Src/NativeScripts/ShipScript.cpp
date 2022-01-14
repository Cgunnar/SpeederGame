#include "pch.hpp"
#include "ShipScript.h"
#include "Input.h"
#include "RenderComponents.h"
#include "PhysicsComponents.h"
#include "Geometry.h"
#include "AssetManager.h"
#include "CollisionDetection.h"
#include <NativeScriptCollection.h>

using namespace rfm;
using namespace rfe;

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
	auto gPad = Input::Get().GamePad().GetState(0);

	if (gPad.IsConnected())
	{
		if (gPad.IsBPressed())
		{
			reset();
			m_docked = true;
		}
		if (gPad.IsAPressed())
		{
			m_docked = false;
		}
		Transform tr = GetComponent<TransformComp>()->transform;
		RigidBody rg = GetComponent<RigidBodyComp>()->rigidBody;

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

		

	}
}


void ShipScript::OnFixedUpdate(float dt)
{
	if (m_docked)
		return;


	AABB aabb = GetComponent<AABBComp>()->aabb;
	Transform transform = GetComponent<TransformComp>()->transform;
	RigidBody rigidBody = GetComponent<RigidBodyComp>()->rigidBody;
	rigidBody.velocity += m_controllInputXYZ * dt;
	rigidBody.angularVelocity += m_controllInputPRY * dt;
	rigidBody.velocity.y -= 9.82 * dt;
	auto aabbPoints = aabb.GetPointsTransformed(transform);

	auto& terrain = EntityReg::GetComponentArray<TerrainScript>().front();
	std::vector<Vector3> points{ aabbPoints.begin(), aabbPoints.end() };
	//points.push_back((points[0] + points[1]) * 0.5f);//8
	//points.push_back((points[1] + points[2]) * 0.5f);//9
	//points.push_back((points[2] + points[3]) * 0.5f);//10
	//points.push_back((points[3] + points[0]) * 0.5f);//11
	//points.push_back((points[8] + points[10]) * 0.5f);//12
	//points.push_back((0.25f*points[8] + 0.75f*points[10]) * 0.5f);//13
	//points.push_back((0.75f*points[8] + 0.25f*points[10]) * 0.5f);//14
	//points.push_back((0.25f*points[9] + 0.75f*points[11]) * 0.5f);//15
	//points.push_back((0.75f*points[9] + 0.25f*points[11]) * 0.5f);//15
	//points.push_back((points[4] + points[5]) * 0.5f);//16
	//points.push_back((points[5] + points[6]) * 0.5f);//17
	//points.push_back((points[6] + points[7]) * 0.5f);//18
	//points.push_back((points[7] + points[4]) * 0.5f);//19
	//points.push_back((points[16] + points[18]) * 0.5f);//20
	//points.push_back((0.25f*points[16] + 0.75f*points[18]) * 0.5f);//21
	//points.push_back((0.75f*points[16] + 0.25f*points[18]) * 0.5f);//22
	//points.push_back((0.25f*points[17] + 0.75f*points[19]) * 0.5f);//23
	//points.push_back((0.75f*points[17] + 0.25f*points[19]) * 0.5f);//24
	//points.push_back((0.25f*points[0] + 0.75f*points[2]) * 0.5f);//25
	//points.push_back((0.75f*points[0] + 0.25f*points[2]) * 0.5f);//26
	//points.push_back((0.25f*points[1] + 0.75f*points[3]) * 0.5f);//27
	//points.push_back((0.75f*points[1] + 0.25f*points[3]) * 0.5f);//28
	//points.push_back((0.25f * points[4] + 0.75f * points[6]) * 0.5f);//29
	//points.push_back((0.75f * points[4] + 0.25f * points[6]) * 0.5f);//30
	//points.push_back((0.25f * points[5] + 0.75f * points[7]) * 0.5f);//31
	//points.push_back((0.75f * points[5] + 0.25f * points[7]) * 0.5f);//32




	std::vector<ConstraintInfo> constraints;
	for (auto& p : points)
	{
		ConstraintInfo c;

		Triangle tri = terrain.GetTriangleAtPos(p);
		if (!tri) return;
		Plane plane = Plane(tri.normal, tri[0]);
		auto colPoint = colDetect::PlaneVSPoints(plane, { p });
		if (colPoint.empty()) continue;
		c.cp = colPoint.front();
		c.tangent = normalize(tri[1] - tri[0]);
		c.biTangent = cross(c.cp.normal, c.tangent);

		constraints.push_back(c);
	}

	const int kMax = 20;
	Matrix3 invI = inverse(rigidBody.momentOfInertia);
	float invMass = 1.0f / rigidBody.mass;
	
	Vector3 v = rigidBody.velocity;
	Vector3 w = rigidBody.angularVelocity;
	float biasFactor = 0.03f * static_cast<float>(kMax) / dt;
	for (int k = 0; k < kMax; k++)
	{
		for (auto& c : constraints)
		{
			Vector3 normal = c.cp.normal;
			Vector3 tangent = c.tangent;
			Vector3 biTangent = c.biTangent;
			Vector3 r = c.cp.pointRealPosition - transform.getTranslation();
			Vector3 constraint = rigidBody.velocity + cross(r, rigidBody.angularVelocity);
			float constraintN = dot(constraint, normal);
			float constraintT = dot(constraint, tangent);
			float constraintB = dot(constraint, biTangent);
			float eMassN = invMass * 0.8f + dot(normal, cross(r, (invI * cross(normal, r))));
			float eMassT = invMass + dot(tangent, cross(r, (invI * cross(tangent, r))));
			float eMassB = invMass + dot(biTangent, cross(r, (invI * cross(biTangent, r))));
			float b = biasFactor * std::max(c.cp.penetration - 0.02f, 0.0f);
			b = std::min(b, abs(constraintN));
			float pCorrectedN = (-constraintN + b) / eMassN;
			float pCorrectedT = (-constraintT) / eMassT;
			float pCorrectedB = (-constraintB) / eMassB;

			float oldP = c.sumP.n;
			c.sumP.n = std::max(c.sumP.n + pCorrectedN, 0.0f);
			pCorrectedN = c.sumP.n - oldP;
			rigidBody.velocity += invMass * pCorrectedN * normal;
			rigidBody.angularVelocity += invI * pCorrectedN * cross(normal, r);

			oldP = c.sumP.t;
			c.sumP.t = std::clamp(c.sumP.t + pCorrectedT, -m_friction * c.sumP.n, m_friction * c.sumP.n);
			pCorrectedT = c.sumP.t - oldP;
			rigidBody.velocity += invMass * pCorrectedT * tangent;
			rigidBody.angularVelocity += invI * pCorrectedT * cross(tangent, r);

			oldP = c.sumP.b;
			c.sumP.b = std::clamp(c.sumP.b + pCorrectedB, -m_friction * c.sumP.n, m_friction * c.sumP.n);
			pCorrectedB = c.sumP.b - oldP;
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