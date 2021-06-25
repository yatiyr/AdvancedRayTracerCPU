#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <Structures.h>
#include <Object.h>

class Light
{
public:

    virtual bool ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                                       const IntersectionReport& report, bool backfaceCulling,
                                       float time, std::vector<Object *>& objectPointerVector) = 0;
    
    virtual glm::vec3 ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                             const float& phongExponent, const IntersectionReport& report,
                                             float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                             bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, float gamma) = 0;

    int ApplyTextures(const IntersectionReport& report, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance)
    {
            if(report.diffuseActive)
            {
                if(report.replaceAll)
                {
                    return 1;
                }
                else if(report.texDiffuseKdMode == 1)
                {
                    diffuseReflectance = report.texDiffuseReflectance;
                }
                else if(report.texDiffuseKdMode == 2)
                {
                    diffuseReflectance = (diffuseReflectance + report.texDiffuseReflectance);
                    diffuseReflectance.x /= 2;
                    diffuseReflectance.y /= 2;
                    diffuseReflectance.z /= 2;
                }
            }

            if(report.specularActive)
            {
                if(report.texSpecularKdMode == 1)
                {
                    specularReflectance = report.texSpecularReflectance;
                }
                else if(report.texSpecularKdMode == 2)
                {
                    specularReflectance = (specularReflectance + report.texSpecularReflectance);
                    specularReflectance.x /= 2;
                    specularReflectance.y /= 2;
                    specularReflectance.z /= 2;
                }
            }

            // Add emisssion later
            if(report.emissionActive)
            {

            }

            return 0;
    }
};


#endif