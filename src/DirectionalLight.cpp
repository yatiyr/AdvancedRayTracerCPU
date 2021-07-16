#include <DirectionalLight.h>



bool DirectionalLight::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                                             const IntersectionReport& report, bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector)
{
    glm::vec3 dir       = -this->direction;
    glm::vec3 origin    = report.intersection + shadowRayEpsilon*report.normal;
        
    Ray ray(origin, dir);
    ray.time = time;

    for(auto object : objectPointerVector)
    {
        IntersectionReport r;
        if(object->Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, backfaceCulling))
            return true;
    }

    return false;
}


glm::vec3 DirectionalLight::ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                                   const float& phongExponent, const IntersectionReport& report,
                                                   float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                                   bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
{

    glm::vec3 result = glm::vec3(0.0);

    if(ShadowRayIntersection(tmin, tmax, intersectionTestEpsilon, shadowRayEpsilon, report, backfaceCulling, ray.time, objectPointerVector))
    {
        return glm::vec3(0.0);
    }
    else
    {
        int applyTex = ApplyTextures(report, diffuseReflectance, specularReflectance);

        if(applyTex == 1)
            return report.texDiffuseReflectance;


        glm::vec3 wi = -direction;

        glm::vec3 diffuseReflectanceU  = diffuseReflectance;
        glm::vec3 specularReflectanceU = specularReflectance;

        // Diffuse Calculation
        if(degammaFlag)
        {
            diffuseReflectanceU.x = std::pow(diffuseReflectance.x,gamma);
            diffuseReflectanceU.y = std::pow(diffuseReflectance.y,gamma);
            diffuseReflectanceU.z = std::pow(diffuseReflectance.z,gamma);

            specularReflectanceU.x = std::pow(specularReflectance.x,gamma);
            specularReflectanceU.y = std::pow(specularReflectance.y,gamma);   
            specularReflectanceU.z = std::pow(specularReflectance.z,gamma); 
        }   

        glm::vec3 brdfComponent = computeF(ray, wi, 
                                           diffuseReflectanceU,
                                           specularReflectanceU,
                                           phongExponent,
                                           report,
                                           hasBRDF,
                                           brdf,
                                           refractiveIndex,
                                           absorbtionIndex);



        result += brdfComponent * 
                  std::max(0.0f, glm::dot(wi, report.normal)) *
                  (radiance);
                  
    }

    return result;
   
}  