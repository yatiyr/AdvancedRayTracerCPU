#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__

#include <Light.h>

class SpotLight : public Light
{
private:
    float GetFollowFactor(float theta);

public:
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 intensity;

    // Angles will be converted to radians
    // while creating the struct
    float coverageAngle;
    float falloffAngle;
    float exponent = 4;

    bool ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                               const IntersectionReport& report, bool backfaceCulling,
                               float time, std::vector<Object *>& objectPointerVector);
    
    glm::vec3 ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                     const float& phongExponent, const IntersectionReport& report,
                                     float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                     bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex);

    
};



#endif