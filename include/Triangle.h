#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include <Structures.h>
#include <Object.h>

class Triangle : public Object
{
private:

public:
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;

    glm::vec3 aNormal;
    glm::vec3 bNormal;
    glm::vec3 cNormal;

    glm::vec2 texCoordA;
    glm::vec2 texCoordB;
    glm::vec2 texCoordC;

    size_t materialId;

    glm::vec3 normal;
    float area;
    
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 aNormal, glm::vec3 bNormal, glm::vec3 cNormal);
    
    //Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, size_t materialId);
    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);    
    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool softShadingFlag, const glm::mat4& transformationMatrixTransposed, bool backfaceCulling);
    bool FasterIntersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon);
    
    glm::vec3 GiveCenter() const;
};



#endif