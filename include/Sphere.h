#ifndef __SPHERE_H__
#define __SPHERE_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Structures.h>

class Sphere
{
private:
    size_t materialId;
    glm::vec3 center;
    float radius;

public:

    Sphere(glm::vec3 center, float radius, size_t materialId);
    bool Intersect(const Ray& r, IntersectionReport& report, float tmin, float tmax);

};

#endif