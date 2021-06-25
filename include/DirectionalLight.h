#ifndef __DIRECTIONAL_LIGHT_H__
#define __DIRECTIONAL_LIGHT_H__

#include <Light.h>

class DirectionalLight : public Light
{
public:
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 radiance;

    bool ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                               const IntersectionReport& report, bool backfaceCulling,
                               float time, std::vector<Object *>& objectPointerVector);
    
    glm::vec3 ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                     const float& phongExponent, const IntersectionReport& report,
                                     float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                     bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, float gamma);
};

#endif