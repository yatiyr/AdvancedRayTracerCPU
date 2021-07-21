#ifndef __LIGHT_SPHERE_H__
#define __LIGHT_SPHERE_H__

#include <Light.h>
#include <Sphere.h>
#include <Structures.h>
#include <RandomGenerator.h>
#include <math.h>

class LightSphere : public Light, public Sphere
{
public:
    glm::vec3 radiance;
    RandomGenerator* randomGenerator;
    glm::vec3 randomPosition;
    float cosThetaMax;

    LightSphere(glm::vec3 center, float radius, size_t materialId);
    ~LightSphere();


    bool ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                                       const IntersectionReport& report, bool backfaceCulling,
                                       float time, std::vector<Object *>& objectPointerVector);

    glm::vec3  ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                     const float& phongExponent, const IntersectionReport& report,
                                     float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                     bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex);                                       

    bool SampleRandomPosition(const Ray& ray, const IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);

    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);

    OrthonormalBasis GiveOrthonormalBasis(glm::vec3 direction);
};

#endif