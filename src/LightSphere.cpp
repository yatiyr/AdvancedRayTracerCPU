#include <LightSphere.h>

LightSphere::LightSphere(glm::vec3 center, float radius, size_t materialId) : Sphere(center, radius, materialId)
{
    this->randomGenerator = new RandomGenerator(0.0f, 1.0f);
}


LightSphere::~LightSphere()
{
    //delete randomGenerator;
}

bool LightSphere::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
                                        const IntersectionReport& report, bool backfaceCulling,
                                        float time, std::vector<Object *>& objectPointerVector)
{

    glm::vec3 direction = glm::normalize(randomPosition - report.intersection);
    glm::vec3 origin    = report.intersection + shadowRayEpsilon*report.normal;

    Ray ray(origin, direction);
    ray.time = time;

    float dist = glm::length(randomPosition - report.intersection);

    for(auto object : objectPointerVector)
    {
        IntersectionReport r;
        if(object->Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, backfaceCulling) && r.d < dist && r.hitObject != this)
            return true;
    }

    return false;

}          


glm::vec3 LightSphere::ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                              const float& phongExponent, const IntersectionReport& report,
                                              float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                              bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
{
    bool test = SampleRandomPosition(ray, report, tmin, tmax, intersectionTestEpsilon, backfaceCulling);
    if(!test)
        return glm::vec3(0.0f);

    glm::vec3 result = glm::vec3(0.0f);

    if(ShadowRayIntersection(tmin, tmax, intersectionTestEpsilon, shadowRayEpsilon, report, backfaceCulling, ray.time, objectPointerVector))
    {
        return glm::vec3(0.0);
    }

    else
    {
        int applyTex = ApplyTextures(report, diffuseReflectance, specularReflectance);

        if(applyTex == 1)
        {
            return report.texDiffuseReflectance;
        }

        float lightDistance = glm::length(randomPosition - report.intersection);
        glm::vec3 wi = glm::normalize(randomPosition - report.intersection);
        glm::vec3 l  = - wi;
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

    //glm::vec3 wCenter =  MotionBlurTranslate2(ray.time) * transformationMatrix * glm::vec4(center, 1.0f);
    //float radius       = glm::length(randomPosition - wCenter);
    float sinThetaMax2 = (radius*radius) / (lightDistance*lightDistance);
    float cosThetaMax  = std::sqrt(std::max((float)0, 1 - sinThetaMax2));
    float invProb = 2 * M_PI * (1 - cosThetaMax);

    result += brdfComponent * 
              std::max(0.f, glm::dot(wi, report.normal)) *
              ((radiance) * invProb);
                                    

    }

    return result;  

}

OrthonormalBasis LightSphere::GiveOrthonormalBasis(glm::vec3 direction)
{
    OrthonormalBasis result;
    glm::vec3 nVec = direction;


    if(std::fabs(nVec.x) <= std::fabs(nVec.y) && std::fabs(nVec.x) <= std::fabs(nVec.z))
    {
        nVec.x = 1.0f;
    }
    else if(std::fabs(nVec.y) <= std::fabs(nVec.x) && std::fabs(nVec.y) <= std::fabs(nVec.z))
    {
        nVec.y = 1.0f;
    }
    else if(std::fabs(nVec.z) <= std::fabs(nVec.x) && std::fabs(nVec.z) <= std::fabs(nVec.y))
    {
        nVec.z = 1.0f;
    }

    result.u = glm::normalize(glm::cross(direction, nVec));
    result.v = glm::normalize(glm::cross(direction, result.u));

    return result;    
}

bool LightSphere::SampleRandomPosition(const Ray& ray, const IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling)
{
    glm::mat4 motionBlurTranslationMatrix = MotionBlurTranslate(ray.time);
    glm::mat4 nTMI = transformationMatrixInversed * motionBlurTranslationMatrix;

    glm::vec3 localPoint = (nTMI * glm::vec4(report.intersection, 1.0f));

    glm::vec3 w = this->center - localPoint;
    float d = glm::length(w);
    w = glm::normalize(w);

    float randomVal1 = randomGenerator->Generate();
    float randomVal2 = randomGenerator->Generate();

    OrthonormalBasis onb = GiveOrthonormalBasis(w);

    float sinThetaMax2 = (radius*radius) / (d*d);
    cosThetaMax = std::sqrt(std::max((float)0, 1 - sinThetaMax2));
    float cosTheta = (1 - randomVal1) + randomVal1 * cosThetaMax;
    float sinTheta = std::sqrt(std::max((float)0, 1 - cosTheta * cosTheta));
    float phi   = 2*M_PI*randomVal2;  

    float theta = std::acos(1 - randomVal1 + randomVal1*cosThetaMax);

    glm::vec3 lLocal = w * std::cos(theta) + onb.u * std::sin(theta) * std::cos(phi) + onb.v * std::sin(theta) * std::sin(phi);

    glm::mat4 tMat = MotionBlurTranslate2(ray.time) * transformationMatrix;
    glm::vec3 lWorld = (tMat * glm::vec4(lLocal, 0.0f));
    lWorld = glm::normalize(lWorld);
    glm::vec3 pWorld = report.intersection + intersectionEpsilon*lWorld;

    Ray newRay(pWorld, lWorld);
    IntersectionReport newReport;

    bool test = Intersect(newRay, newReport, tmin, tmax, intersectionEpsilon, backfaceCulling);

    if(test)
    {
        randomPosition = newReport.intersection;
        return true;
    }

    return false; 

}

bool LightSphere::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling)
{
    glm::mat4 motionBlurTranslationMatrix = MotionBlurTranslate(ray.time);

    glm::vec3 newOrigin = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(ray.origin, 1.0f));
    glm::vec3 newDirection = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(ray.direction, 0.0f));
    Ray newRay(newOrigin, newDirection);

    report.intersection = glm::vec3(-FLT_MAX);
    report.d            = FLT_MAX;
    report.isLight      = true;

    float discriminant = pow(glm::dot(newRay.direction, (newRay.origin - center)), 2) -
                         dot(newRay.direction, newRay.direction) * (glm::dot(newRay.origin - center, newRay.origin - center) -
                         radius * radius);

    float t;

    if(discriminant >= 0)
    {
        float t1 = -(glm::dot(newRay.direction, (newRay.origin - center)) + sqrt(discriminant)) / glm::dot(newRay.direction, newRay.direction);
        float t2 = -(glm::dot(newRay.direction, (newRay.origin - center)) - sqrt(discriminant)) / glm::dot(newRay.direction, newRay.direction);

        t = std::min(t1, t2);

        if(t1 * t2 < 0)
            t = std::max(t1, t2);

        if(t > tmin - intersectionEpsilon && t < tmax + intersectionEpsilon)
        {
            report.d            = t;
            report.intersection = ray.origin + t*ray.direction;
            report.hitObject    = this;
            
            float cosThetaMax = std::sqrt(1 - (radius*radius)/(radius*radius));            
            float invProb = 2 * M_PI * (1 - cosThetaMax);

            report.radiance     = radiance * invProb;
            report.materialId   = materialId;
            glm::vec3 normal = (newRay.origin + t*newRay.direction) - center;
            report.normal       = glm::transpose(motionBlurTranslationMatrix)*transformationMatrixInverseTransposed * glm::vec4(normal, 0.0f);
            report.normal = glm::normalize(report.normal);

            glm::vec3 localIntersection = newRay.origin + t*newRay.direction;

            float x = localIntersection.x - this->center.x;
            float y = localIntersection.y - this->center.y;
            float z = localIntersection.z - this->center.z;

            float theta = std::acos(y/this->radius);
            float phi   = std::atan2(z, x);

            report.texCoord.x = (-phi + M_PI) / (2 * M_PI);
            report.texCoord.y = theta / M_PI;            

            return true;
        }
      
    }

    return false;
}