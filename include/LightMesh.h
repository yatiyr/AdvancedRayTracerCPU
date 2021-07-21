#ifndef __LIGHT_MESH_H__
#define __LIGHT_MESH_H__

#include <Light.h>
#include <Mesh.h>
#include <Structures.h>
#include <RandomGenerator.h>
#include <algorithm>
#include <math.h>

class LightMesh : public Light , public Mesh 
{
public:
    glm::vec3 radiance;
    float totalArea;
    std::vector<Triangle> triangleList;
    RandomGenerator* randomGenerator;
    glm::vec3 randomPosition;
    glm::vec3 randomNormal;

    LightMesh(const std::vector<Triangle>& triangleList, size_t materialId, bool softShadingFlag);
    ~LightMesh();


    bool ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                                       const IntersectionReport& report, bool backfaceCulling,
                                       float time, std::vector<Object *>& objectPointerVector);

    glm::vec3  ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                     const float& phongExponent, const IntersectionReport& report,
                                     float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                     bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex);                                       

    void SampleRandomPosition(const Ray& ray, const IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);

    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);
};

#endif