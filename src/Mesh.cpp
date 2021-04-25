#include <Mesh.h>


Mesh::Mesh(const std::vector<Triangle>& triangleList, size_t materialId)
{
    this->bvhRoot = new BVH(triangleList, 0, 200, 0);
    this->materialId = materialId;
}

bool Mesh::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon)
{
    bool test = bvhRoot->Intersect(ray, report, tmin, tmax, intersectionEpsilon);

    report.materialId = materialId;

    return test;
}