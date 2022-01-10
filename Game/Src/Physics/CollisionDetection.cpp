#include "pch.hpp"
#include "CollisionDetection.h"
#include "RfextendedMath.hpp"

using namespace colDetect;
using namespace rfm;

std::vector<CollisionPoint> colDetect::PlaneVSPoints(Plane plane, std::vector<rfm::Vector3> points)
{
    std::vector<CollisionPoint> colPoints;
    for (const auto& p : points)
    {
        float pen = dot(plane.normal, p) + plane.d;
        if (pen <= 0)
        {
            CollisionPoint cp;
            cp.normal = plane.normal;
            cp.penetration = -pen;
            cp.intersectionPoint = p + plane.normal * pen;
            cp.pointRealPosition = p;
            colPoints.push_back(cp);
        }
    }
    std::sort(colPoints.begin(), colPoints.end(), [](auto a, auto b) { return a.penetration > b.penetration; });
    return colPoints;
}
