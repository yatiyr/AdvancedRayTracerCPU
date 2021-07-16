#include <SpotLight.h>

float SpotLight::GetFollowFactor(float theta)
{
    return std::pow((std::cos(theta) - std::cos(coverageAngle/2))/
           (std::cos(falloffAngle/2) - std::cos(coverageAngle/2)),exponent);
}


bool SpotLight::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
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


glm::vec3 SpotLight::ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                            const float& phongExponent, const IntersectionReport& report,
                                            float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                            bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
{

    glm::vec3 result = glm::vec3(0.0);

    if(ShadowRayIntersection(tmin, tmax, intersectionTestEpsilon, shadowRayEpsilon, report, true, ray.time, objectPointerVector))
    {
        return glm::vec3(0.0);
    }
    else
    {

        int applyTex = ApplyTextures(report, diffuseReflectance, specularReflectance);

        if(applyTex == 1)
            return report.texDiffuseReflectance;

        glm::vec3 directionToObject = glm::normalize(report.intersection - position);
        float lightDistance = glm::length(position - report.intersection);
        glm::vec3 wi = glm::normalize(position - report.intersection);
        glm::vec3 h = glm::normalize(wi - ray.direction);

        // Angle between spotLight direction and directionToObject
        float costheta = glm::dot(direction, directionToObject);
        float theta = std::acos(costheta);

        // point is out of spotlight's area
        if(theta >= coverageAngle/2)
        {
            return glm::vec3(0.0);           
        }
        else if(theta < coverageAngle/2 && theta > falloffAngle/2)
        { 
            float followFactor = GetFollowFactor(theta);

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
                    (intensity * followFactor / (lightDistance * lightDistance));           
          
        }
        else if(theta <= falloffAngle/2)
        {

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
                    (intensity / (lightDistance * lightDistance));             

        }
    }

    return result;

}