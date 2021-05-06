#ifndef __MESH_H__
#define __MESH_H__

#include <BVH.h>
#include <vector>
#include <Triangle.h>
#include <Structures.h>
#include <Object.h>

class Mesh : public Object
{
private:
    BVH* bvhRoot;
    int materialId;

    bool softShadingFlag;

public:
    Mesh(const std::vector<Triangle>& triangleList, size_t materialId, bool softShadingFlag);
    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon);
};


#endif