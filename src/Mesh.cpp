#include <Mesh.h>




Mesh::Mesh(const std::vector<Triangle>& triangleList, size_t materialId)
{
    bvhRoot = new BVH(triangleList, 0, 10);
}

bool Mesh::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon)
{
    return bvhRoot->Intersect(ray, report, tmin, tmax, intersectionEpsilon);
}