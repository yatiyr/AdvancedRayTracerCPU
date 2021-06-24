#include <PointLight.h>



bool PointLight::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
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


glm::vec3 PointLight::ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                             const float& phongExponent, const IntersectionReport& report,
                                             float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                             bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector)
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

        float lightDistance = glm::length(position - report.intersection);
        glm::vec3 wi = glm::normalize(position - report.intersection);

        // Diffuse Calculation
        result += diffuseReflectance * 
                std::max(0.0f, glm::dot(wi, report.normal)) *
                (intensity / (lightDistance * lightDistance));

        // Specular Calculation
        glm::vec3 h = glm::normalize(wi - ray.direction);

        result += specularReflectance *
                std::pow(std::max(0.0f, glm::dot(report.normal, h)), phongExponent) *
                (intensity / (lightDistance * lightDistance));
    }

    return result;   
}                                        