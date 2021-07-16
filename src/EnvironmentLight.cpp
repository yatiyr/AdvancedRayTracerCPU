#include <EnvironmentLight.h>

void EnvironmentLight::RejectionSampling(glm::vec3 normal)
{
    glm::vec3 resultVec(0.0);

    while(glm::length(resultVec) > 1 || glm::dot(resultVec, normal) <= 0)
    {
        float randomX = randomNumberGenerator->Generate()*2 - 1;
        float randomY = randomNumberGenerator->Generate()*2 - 1;
        float randomZ = randomNumberGenerator->Generate()*2 - 1;

        resultVec = glm::vec3(randomX, randomY, randomZ);
    }

    randomDirection = glm::normalize(resultVec);

}

EnvironmentLight::EnvironmentLight()
{
    randomNumberGenerator = new RandomGenerator(0.0f, 1.0f);
}

bool EnvironmentLight::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                                             const IntersectionReport& report, bool backfaceCulling,
                                             float time, std::vector<Object *>& objectPointerVector)
{
    glm::vec3 dir       = randomDirection;
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

glm::vec3 EnvironmentLight::ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                                   const float& phongExponent, const IntersectionReport& report,
                                                   float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                                   bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
{
    RejectionSampling(report.normal);
    float theta = std::acos(glm::dot(randomDirection, glm::vec3(0.0, 1.0, 0.0)));
    float phi   = std::atan2(glm::dot(randomDirection, glm::vec3(0.0, 0.0, 1.0)), glm::dot(randomDirection, glm::vec3(1.0, 0.0, 0.0)));

    float u = (-phi + M_PI)/(2*M_PI);
    float v = theta/M_PI;

    glm::vec3 radiance = hdrTexture.Fetch(u,v);
    radiance.x *= 2*M_PI;
    radiance.y *= 2*M_PI;
    radiance.z *= 2*M_PI;

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


        glm::vec3 wi = randomDirection;

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