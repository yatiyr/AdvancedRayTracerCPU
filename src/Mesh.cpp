#include <Mesh.h>


Mesh::Mesh(const std::vector<Triangle>& triangleList, size_t materialId, bool softShadingFlag)
{
    this->bvhRoot = new BVH(triangleList, 0, 200, 0);
    this->materialId = materialId;
    this->softShadingFlag = softShadingFlag;
}

bool Mesh::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon)
{
    bool test = bvhRoot->Intersect(ray, report, tmin, tmax, intersectionEpsilon, softShadingFlag);

    report.materialId = materialId;

    return test;
}