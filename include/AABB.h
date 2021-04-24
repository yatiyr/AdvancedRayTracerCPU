#ifndef __AABB_H__
#define __AABB_H__

#include <Structures.h>
#include <Triangle.h>

class AABB
{
private:

public:

    float xmin;
    float ymin;
    float zmin;
    float xmax;
    float ymax;
    float zmax;
    
    AABB(const std::vector<Triangle>& triangleList);
    bool Intersect(const Ray& r, float tmin, float tmax);
};


#endif