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
                                             bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex) = 0;

    glm::vec3 computeF(const Ray& ray, glm::vec3& wi, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance, const float& phongExponent, const IntersectionReport& report,
                   bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
    {

        glm::vec3 result(0.0);

        glm::vec3 halfVector = glm::normalize(wi - ray.direction);

        glm::vec3 reflectionWrtNormal = glm::normalize(glm::reflect(wi, report.normal));

        float cosThetaI = glm::dot(wi, report.normal);

        if(cosThetaI <= 0)
            return glm::vec3(0.0);

        float cosAlphaR = std::max(0.0f, glm::dot(-ray.direction, reflectionWrtNormal));

        float cosAlphaH = std::max(0.0f, glm::dot(halfVector, report.normal));

        float cosBeta   = std::max(0.0f, glm::dot(-ray.direction, halfVector));

        if(hasBRDF)
        {
            if(brdf.type == BRDFType::ORIGINAL_PHONG)
            {
                result += diffuseReflectance + specularReflectance * std::pow(cosAlphaR, brdf.exponent) / cosThetaI;               
            }
            else if(brdf.type == BRDFType::ORIGINAL_BLINN_PHONG)
            {
                result += diffuseReflectance + specularReflectance * std::pow(cosAlphaH, brdf.exponent) / cosThetaI;
            }
            else if(brdf.type == BRDFType::MODIFIED_PHONG)
            {
                if(brdf.normalized)
                    result += diffuseReflectance /(float(M_PI)) + specularReflectance * ((brdf.exponent + 2)/(float(2*M_PI))) * std::pow(cosAlphaR, brdf.exponent);
                else
                    result += diffuseReflectance + specularReflectance * std::pow(cosAlphaR, brdf.exponent);
            }
            else if(brdf.type == BRDFType::MODIFIED_BLINN_PHONG)
            {
                if(brdf.normalized)
                    result += diffuseReflectance /(float(M_PI)) + specularReflectance * ((brdf.exponent + 8)/(float(8*M_PI))) * std::pow(cosAlphaH, brdf.exponent);
                else
                    result += diffuseReflectance + specularReflectance * std::pow(cosAlphaH, brdf.exponent);
            }
            else if(brdf.type == BRDFType::TORRANCE_SPARROW)
            {
                // Compute D(alpha) - Blinn's distribution is used
                float probability = ((brdf.exponent + 2) / (2*M_PI)) * std::pow(cosAlphaH, brdf.exponent);

                // Compute Geometry Term
                float nDotWo = std::max(0.0f, glm::dot(report.normal, -ray.direction));
                float part1 = (2*cosAlphaH * nDotWo) / cosBeta;
                float part2 = (2*cosAlphaH * cosThetaI) / cosBeta;
                float geometryTerm = std::min(1.0f, std::min(part1, part2));

                // Compute Fresnel reflectance
                float r0 = std::pow(refractiveIndex - 1, 2) / std::pow(refractiveIndex + 1, 2);
                float fresnel = r0 + (1 - r0) * std::pow(1 - cosBeta, 5);

                glm::vec3 diffusePart = diffuseReflectance;

                if(brdf.kdfresnel)
                    diffusePart = (1 - fresnel) * diffusePart;

                result += diffusePart * (float(M_PI)) + specularReflectance * probability * fresnel * geometryTerm / (4 * cosThetaI * nDotWo);

            }            
        }
        else
        {
            result += diffuseReflectance + specularReflectance * std::pow(cosAlphaH, phongExponent) / cosThetaI;
        }

        return result;
    }                   

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