#include <AreaLight.h>



AreaLight::AreaLight(glm::vec3 position, glm::vec3 radiance, glm::vec3 normal, float extent)
{
    this->normal   = normal;
    this->position = position;
    this->radiance = radiance;
    this->extent   = extent;

    glm::vec3 nBar = normal;

    float absX = std::fabs(nBar.x);
    float absY = std::fabs(nBar.y);
    float absZ = std::fabs(nBar.z);

    if(absX <= absY && absX <= absZ)
    {
        nBar.x = 1.0f;
    }
    else if(absY <= absX && absY <= absZ)
    {
        nBar.y = 1.0f;
    }
    else if(absZ <= absX && absZ <= absY)
    {
        nBar.z = 1.0f;
    }

    this->u = glm::normalize(glm::cross(nBar, normal));
    this->v = glm::normalize(glm::cross(normal, u));

    this->areaLightPositionGenerator = new RandomGenerator(-0.5f, 0.5f);    
}

bool AreaLight::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                                      const IntersectionReport& report, bool backfaceCulling,
                                      float time, std::vector<Object *>& objectPointerVector)
{
    glm::vec3 direction = glm::normalize(position - report.intersection);
    glm::vec3 origin    = report.intersection + shadowRayEpsilon*report.normal;

    Ray ray(origin, direction);
    ray.time = time;

    float dist = glm::length(position - report.intersection);

    for(auto object : objectPointerVector)
    {
        IntersectionReport r;
        if(object->Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, backfaceCulling) && r.d < dist)
            return true;
    }

    return false;
}


glm::vec3 AreaLight::ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                            const float& phongExponent, const IntersectionReport& report,
                                            float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                            bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
{
    glm::vec3 result = glm::vec3(0.0);

    float randomOffsetU = areaLightPositionGenerator->Generate();
    float randomOffsetV = areaLightPositionGenerator->Generate();
    glm::vec3 randomPoint = position + extent*(randomOffsetU*u + randomOffsetV*v);

    if(ShadowRayIntersection(tmin, tmax, intersectionTestEpsilon, shadowRayEpsilon, report, backfaceCulling, ray.time, objectPointerVector))
    {
        return glm::vec3(0.0);
    }
    else
    {
        int applyTex = ApplyTextures(report, diffuseReflectance, specularReflectance);

        if(applyTex == 1)
            return report.texDiffuseReflectance;

            float lightDistance = glm::length(randomPoint - report.intersection);
            glm::vec3 wi = glm::normalize(randomPoint - report.intersection);
            glm::vec3 l = -wi;

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
                ((radiance * std::fabs(glm::dot(l, normal)) * extent * extent)/(lightDistance*lightDistance));
      
    }
    return result;
}   
