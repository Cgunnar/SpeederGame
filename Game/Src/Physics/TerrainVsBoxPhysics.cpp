#include "pch.hpp"
#include "TerrainVsBoxPhysics.h"
#include "RimfrostMath.hpp"
#include "PhysicsComponents.h"
#include "StandardComponents.h"
#include "TerrainScript.h"
#include "PhysicsTypes.h"
#include "CollisionDetection.h"

using namespace std;
using namespace rfm;
using namespace rfe;

namespace phySys
{
	vector<ConstraintInfo> FindCollision(const AABB& aabb, const Transform &transform, const RigidBody& rigidBody);
	void RespondToCollision(float dt, vector<ConstraintInfo>& constraints, Transform& transform, RigidBody& rigidBody);

	void FindAndResolveTerrainBoxCollision(float dt)
	{
		auto& rigidbodys = EntityReg::GetComponentArray<RigidBodyComp>();
		for (auto& r : rigidbodys)
		{
			EntityID id = r.GetEntityID();
			if (auto aabbComp = EntityReg::GetComponent<AABBComp>(id); aabbComp)
			{
				assert(EntityReg::GetComponent<TransformComp>(id));
				Transform& transform = EntityReg::GetComponent<TransformComp>(id)->transform;
				AABB aabb = aabbComp->aabb;

				vector<ConstraintInfo> constraints = FindCollision(aabb, transform, r.rigidBody);
				RespondToCollision(dt, constraints, transform, r.rigidBody);
			}
		}
	}

	void RespondToCollision(float dt, vector<ConstraintInfo>& constraints, Transform& transform, RigidBody& rigidBody)
	{
		const int kMax = 8;
		Matrix3 rot = (Matrix3)transform.getRotationMatrix();
		Matrix I = transpose(rot) * rigidBody.momentOfInertia * rot; //transform moment of inertia to worldSpace
		Matrix3 invI = inverse(I);
		float invMass = 1.0f / rigidBody.mass;

		float biasFactor = 0.03f * static_cast<float>(kMax) / dt;
		for (int k = 0; k < kMax; k++)
		{
			for (auto& c : constraints)
			{
				Vector3 constraint = rigidBody.velocity + cross(c.r, rigidBody.angularVelocity);
				float constraintN = dot(constraint, c.normal);
				float constraintT = dot(constraint, c.tangent);
				//0.8 is just some hardcoded cof to make it bounce more
				float eMassN = invMass * 0.8f + dot(c.normal, cross(c.r, (invI * cross(c.normal, c.r))));
				float eMassT = invMass + dot(c.tangent, cross(c.r, (invI * cross(c.tangent, c.r))));
				float b = biasFactor * std::max(c.pen - 0.02f, 0.0f);
				b = std::min(b, abs(constraintN));
				float pCorrectedN = (-constraintN + b) / eMassN;
				float pCorrectedT = (-constraintT) / eMassT;

				float oldP = c.sumPn;
				c.sumPn = std::max(c.sumPn + pCorrectedN, 0.0f);
				pCorrectedN = c.sumPn - oldP;
				rigidBody.velocity += invMass * pCorrectedN * c.normal;
				rigidBody.angularVelocity += invI * pCorrectedN * cross(c.normal, c.r);

				oldP = c.sumPt;
				c.sumPt = std::clamp(c.sumPt + pCorrectedT, -rigidBody.frictionCof * c.sumPn, rigidBody.frictionCof * c.sumPn);
				pCorrectedT = c.sumPt - oldP;
				rigidBody.velocity += invMass * pCorrectedT * c.tangent;
				rigidBody.angularVelocity += invI * pCorrectedT * cross(c.tangent, c.r);
			}
		}

		
	}

	vector<ConstraintInfo> FindCollision(const AABB& aabb, const Transform& transform, const RigidBody& rigidBody)
	{
		auto aabbPoints = aabb.GetPointsTransformed(transform);
		auto& terrain = EntityReg::GetComponentArray<TerrainScript>().front();
		std::vector<Vector3> points{ aabbPoints.begin(), aabbPoints.end() };
		points.push_back((points[0] + points[1]) * 0.5f);//8
		points.push_back((points[1] + points[2]) * 0.5f);//9
		points.push_back((points[2] + points[3]) * 0.5f);//10
		points.push_back((points[3] + points[0]) * 0.5f);//11
		points.push_back((points[8] + points[10]) * 0.5f);//12
		points.push_back(0.25f * points[8] + 0.75f * points[10]);//13
		points.push_back(0.75f * points[8] + 0.25f * points[10]);//14
		points.push_back(0.25f * points[9] + 0.75f * points[11]);//15
		points.push_back(0.75f * points[9] + 0.25f * points[11]);//15
		points.push_back((points[4] + points[5]) * 0.5f);//16
		points.push_back((points[5] + points[6]) * 0.5f);//17
		points.push_back((points[6] + points[7]) * 0.5f);//18
		points.push_back((points[7] + points[4]) * 0.5f);//19
		points.push_back((points[16] + points[18]) * 0.5f);//20
		points.push_back(0.25f * points[16] + 0.75f * points[18]);//21
		points.push_back(0.75f * points[16] + 0.25f * points[18]);//22
		points.push_back(0.25f * points[17] + 0.75f * points[19]);//23
		points.push_back(0.75f * points[17] + 0.25f * points[19]);//24
		points.push_back(0.25f * points[0] + 0.75f * points[2]);//25
		points.push_back(0.75f * points[0] + 0.25f * points[2]);//26
		points.push_back(0.25f * points[1] + 0.75f * points[3]);//27
		points.push_back(0.75f * points[1] + 0.25f * points[3]);//28
		points.push_back(0.25f * points[4] + 0.75f * points[6]);//29
		points.push_back(0.75f * points[4] + 0.25f * points[6]);//30
		points.push_back(0.25f * points[5] + 0.75f * points[7]);//31
		points.push_back(0.75f * points[5] + 0.25f * points[7]);//32


		std::vector<ConstraintInfo> constraints;
		for (auto& p : points)
		{
			ConstraintInfo c;
			Triangle tri = terrain.GetTriangleAtPos(p);
			if (!tri) return vector<ConstraintInfo>();
			Plane plane = Plane(tri.normal, tri[0]);
			CollisionPoint colPoint = colDetect::PlaneVSPoint(plane, p);
			if (!colPoint) continue;
			c.pen = colPoint.penetration;
			c.normal = colPoint.normal;
			c.r = colPoint.pointRealPosition - transform.getTranslation();
			Vector3 constraint = rigidBody.velocity + cross(c.r, rigidBody.angularVelocity);
			c.tangent = (constraint - dot(constraint, c.normal) * c.normal).length() > 0.0f ? normalize(constraint - dot(constraint, c.normal) * c.normal) : normalize(tri[1] - tri[0]);
			constraints.push_back(c);
		}
		return constraints;
	}
};



