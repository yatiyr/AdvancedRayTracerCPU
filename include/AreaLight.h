#ifndef __AREA_LIGHT_H__
#define __AREA_LIGHT_H__

#include <Light.h>
#include <Structures.h>
#include <RandomGenerator.h>

class AreaLight : public Light
{
public:
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 radiance;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec3 u;
    alignas(16) glm::vec3 v;
    float extent;

    RandomGenerator* areaLightPositionGenerator;

    AreaLight(glm::vec3 position, glm::vec3 radiance, glm::vec3 normal, float extent);
    bool ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                               const IntersectionReport& report, bool backfaceCulling,
                               float time, std::vector<Object *>& objectPointerVector);
    
    glm::vec3 ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                     const float& phongExponent, const IntersectionReport& report,
                                     float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                     bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex);    
};

#endif