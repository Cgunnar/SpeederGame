#include "pch.hpp"
#include "BoxPhysicsTestScript.h"
#include "CollisionDetection.h"
#include "PhysicsComponents.h"
#include "ShipContollerScript.h"
#include "TerrainScript.h"
using namespace rfm;
using namespace rfe;
using namespace std;

void BoxPhysicsTestScript::OnStart()
{

}

void BoxPhysicsTestScript::OnUpdate(float dt)
{

}

void BoxPhysicsTestScript::OnFixedUpdate(float dt)
{
	if (EntityReg::GetComponentArray<ShipContollerScript>().front().m_docked)
		return;
	Plane plane0 = Plane({ 0,1,0 }, 0);
	AABB aabb = GetComponent<AABBComp>()->aabb;
	Transform transform = GetComponent<TransformComp>()->transform;
	RigidBody rigidBody = GetComponent<RigidBodyComp>()->rigidBody;

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



	std::vector<ConstraintInfo> constraints;
	for (auto& p : points)
	{
		ConstraintInfo c;

		Triangle tri = terrain.GetTriangleAtPos(p);
		Plane plane = Plane(tri.normal, tri[0]);
		auto colPoint = colDetect::PlaneVSPoints(plane, { p });
		if (colPoint.empty()) continue;
		c.cp = colPoint.front();
		c.tangent = normalize(tri[1] - tri[0]);
		c.biTangent = cross(c.cp.normal, c.tangent);

		constraints.push_back(c);
	}

	//auto pointsUnderPlane = colDetect::PlaneVSPoints(plane0, { aabbPoints.begin(), aabbPoints.end() });
	std::vector<sumP> sumP;
	//sumP.resize(pointsUnderPlane.size());
	const int kMax = 40;
	Matrix3 invI = inverse(rigidBody.momentOfInertia);
	float invMass = 1.0f / rigidBody.mass;
	rigidBody.velocity.y -= 9.82 * dt;
	Vector3 v = rigidBody.velocity;
	Vector3 w = rigidBody.angularVelocity;
	float biasFactor = 0.6f / (dt / static_cast<float>(kMax));
	for (int k = 0; k < kMax; k++)
	{
		//for (int i = 0; i < pointsUnderPlane.size(); i++)
		for (auto& c : constraints)
		{
			Vector3 normal = c.cp.normal;
			Vector3 tangent = c.tangent;
			Vector3 biTangent = c.biTangent;
			Vector3 r = c.cp.pointRealPosition - transform.getTranslation();
			Vector3 constaint = rigidBody.velocity + cross(r, rigidBody.angularVelocity);
			float constraintN = dot(constaint, normal);
			float constraintT = dot(constaint, tangent);
			float constraintB = dot(constaint, biTangent);
			float eMassN = invMass + dot(normal, cross(r, (invI * cross(normal, r))));
			float eMassT = invMass + dot(tangent, cross(r, (invI * cross(tangent, r))));
			float eMassB = invMass + dot(biTangent, cross(r, (invI * cross(biTangent, r))));
			float b = biasFactor * std::max(c.cp.penetration - 0.015f, 0.0f);
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
