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

    glm::vec3 bounds[2];
    
    AABB(const std::vector<Triangle>& triangleList);
    bool Intersect(const Ray& r);
    bool Intersect2(const Ray& r, float t0, float t1);
};


#endif