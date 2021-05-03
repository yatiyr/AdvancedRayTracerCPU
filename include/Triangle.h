#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include <Structures.h>

class Triangle
{
private:

public:
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;

    glm::vec3 aNormal;
    glm::vec3 bNormal;
    glm::vec3 cNormal;

    size_t materialId;

    glm::vec3 normal;
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 aNormal, glm::vec3 bNormal, glm::vec3 cNormal);
    
    //Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, size_t materialId);
    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool softShadingFlag);
    bool FasterIntersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon);
    
    glm::vec3 GiveCenter() const;
};



#endif