#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <glm/mat4x4.hpp>
#include <Structures.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Texture.h>

class Object
{
public:
    glm::mat4 transformationMatrix;
    glm::mat4 transformationMatrixTransposed;
    glm::mat4 transformationMatrixInversed;
    glm::mat4 transformationMatrixInverseTransposed;

    glm::vec3 translationVector;

    Texture *diffuseMap = nullptr;
    Texture *specularMap = nullptr;
    Texture *normalMap = nullptr;
    Texture *bumpMap = nullptr;
    Texture *emissionMap = nullptr;
    Texture *roughnessMap = nullptr;

    size_t materialId;

    virtual bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling) = 0;

    glm::mat4 MotionBlurTranslate(float time)
    {
        if(translationVector.x == 0 &&
           translationVector.y == 0 &&
           translationVector.z == 0)
           return glm::mat4(1.0f);

        glm::vec3 currentTranslation = time * translationVector;

        return glm::inverse(glm::translate(glm::mat4(1.0f),currentTranslation));

    }

    glm::mat4 MotionBlurTranslate2(float time)
    {
        if(translationVector.x == 0 &&
           translationVector.y == 0 &&
           translationVector.z == 0)
           return glm::mat4(1.0f);

        glm::vec3 currentTranslation = time * translationVector;

        return glm::translate(glm::mat4(1.0f),currentTranslation);        
    }

    float ColorDistance(glm::vec3 c1, glm::vec3 c2)
    {
        long rmean = ((long)c1.x + (long)c2.x) / 2;
        long r = ((long)c1.x - (long)c2.x);
        long g = (long)c1.y - (long)c2.y;
        long b = (long)c1.z - (long)c2.z;
        return std::sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8))/5;
    }

    int ApplyTex(const IntersectionReport& report, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance)
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

    glm::vec3 getF(const Ray& ray, glm::vec3& wi, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance, const float& phongExponent, const IntersectionReport& report,
                   bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
    {

        glm::vec3 result(0.0);

        glm::vec3 halfVector = glm::normalize(wi - ray.direction);

        glm::vec3 reflectionWrtNormal = glm::normalize(glm::reflect(wi, report.normal));

        float cosThetaI = glm::dot(wi, report.normal);

        if(cosThetaI <= 0)
            return glm::vec3(0.0);

        float cosAlphaR = std::max(0.0f, glm::dot(-ray.direction, -reflectionWrtNormal));

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

                float aI = absorbtionIndex;
                float rI = refractiveIndex;

                float rS = ((rI*rI + aI*aI) - 2*rI*cosThetaI + (cosThetaI*cosThetaI))/
                        ((rI*rI + aI*aI) + 2*rI*cosThetaI + (cosThetaI*cosThetaI));

                float rP = ((rI*rI + aI*aI)*(cosThetaI*cosThetaI) - 2*rI*cosThetaI + 1)/
                        ((rI*rI + aI*aI)*(cosThetaI*cosThetaI) + 2*rI*cosThetaI + 1);

                float reflectionRatio = (rS + rP)/2; 

                float r0 = std::pow(refractiveIndex - 1, 2) / std::pow(refractiveIndex + 1, 2);
                float fresnel = reflectionRatio;//r0 + (1 - r0) * std::pow(1 - cosBeta, 5);

                glm::vec3 diffusePart = diffuseReflectance;

                if(brdf.kdfresnel)
                    diffusePart = (1 - fresnel) * diffusePart;

                result += diffusePart / (float(M_PI)) + specularReflectance * probability * fresnel * geometryTerm / (4 * cosThetaI * nDotWo);

            }            
        }
        else
        {
            result += diffuseReflectance + specularReflectance * std::pow(cosAlphaH, phongExponent) / cosThetaI;
        }

        return result;
    }


    glm::vec3 getReflectance(const Ray& ray, glm::vec3& wi, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                             const float& phongExponent, const IntersectionReport& report,
                             bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
    {
        glm::vec3 result = glm::vec3(0.0);

        int applyTex = ApplyTex(report, diffuseReflectance, specularReflectance);

        if(applyTex == 1)
        {
            return report.texDiffuseReflectance;
        }

        glm::vec3 diffuseReflectanceU  = diffuseReflectance;
        glm::vec3 specularReflectanceU = specularReflectance;

        if(degammaFlag)
        {
            diffuseReflectanceU.x = std::pow(diffuseReflectance.x,gamma);
            diffuseReflectanceU.y = std::pow(diffuseReflectance.y,gamma);
            diffuseReflectanceU.z = std::pow(diffuseReflectance.z,gamma);

            specularReflectanceU.x = std::pow(specularReflectance.x,gamma);
            specularReflectanceU.y = std::pow(specularReflectance.y,gamma);   
            specularReflectanceU.z = std::pow(specularReflectance.z,gamma); 
        }

        glm::vec3 brdfComponent = getF(ray, wi, 
                                           diffuseReflectanceU,
                                           specularReflectanceU,
                                           phongExponent,
                                           report,
                                           hasBRDF,
                                           brdf,
                                           refractiveIndex,
                                           absorbtionIndex);

        result += brdfComponent * std::max(0.0f, glm::dot(wi, report.normal));

        return result;

    }                             


};

#endif  /* __OBJECT_H__ */