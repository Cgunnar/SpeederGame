#include "pch.hpp"
#include "boundingVolumes.h"

AABB AABB::Merge(AABB a, AABB b)
{
    if (a.max.x < b.max.x) a.max.x = b.max.x;
    if (a.max.y < b.max.y) a.max.y = b.max.y;
    if (a.max.z < b.max.z) a.max.z = b.max.z;

    if (a.min.x > b.min.x) a.min.x = b.min.x;
    if (a.min.y > b.min.y) a.min.y = b.min.y;
    if (a.min.z > b.min.z) a.min.z = b.min.z;

    return a;
}
