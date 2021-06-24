#ifndef __ENVIRONMENT_LIGHT_H__
#define __ENVIRONMENT_LIGHT_H__

#include <Light.h>
#include <Texture.h>
#include <RandomGenerator.h>

class EnvironmentLight : public Light
{
private:
    void RejectionSampling(glm::vec3 normal);
public:
    Texture hdrTexture;
    RandomGenerator* randomNumberGenerator;
    glm::vec3 randomDirection;

    EnvironmentLight();

    bool ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                               const IntersectionReport& report, bool backfaceCulling,
                               float time, std::vector<Object *>& objectPointerVector);
    
    glm::vec3 ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                     const float& phongExponent, const IntersectionReport& report,
                                     float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                     bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector);     

};


#endif