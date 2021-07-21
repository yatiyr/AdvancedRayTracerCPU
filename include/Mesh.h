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

public:
    BVH* bvhRoot;
    bool softShadingFlag;
    Mesh(const std::vector<Triangle>& triangleList, size_t materialId, bool softShadingFlag);
    virtual bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);
};


#endif