#ifndef __SPHERE_H__
#define __SPHERE_H__

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Structures.h>
#include <Object.h>
#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

class Sphere : public Object
{
private:
    size_t materialId;
    glm::vec3 center;
    float radius;

    bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float& x1);

public:

    Sphere(glm::vec3 center, float radius, size_t materialId);
    bool Intersect(const Ray& r, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);

};

#endif