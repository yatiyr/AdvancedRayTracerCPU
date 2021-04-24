#ifndef __MESH_H__
#define __MESH_H__

#include <BVH.h>
#include <vector>
#include <Triangle.h>
#include <Structures.h>


class Mesh
{
private:
    BVH* bvhRoot;
    int materialId;

public:
    Mesh(const std::vector<Triangle>& triangleList, size_t materialId);
    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon);
};


#endif