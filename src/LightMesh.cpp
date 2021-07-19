#include <LightMesh.h>


LightMesh::LightMesh(const std::vector<Triangle>& triangleList, size_t materialId, bool softShadingFlag) : Mesh(triangleList, materialId, softShadingFlag)
{
    this->randomGenerator =  new RandomGenerator(0.0f, 1.0f);
    this->triangleList    = triangleList;
    this->totalArea       = 0.0f;

    // we sort triangle list according to their areas in ascending order
    std::sort(this->triangleList.begin(), this->triangleList.end(),
    [](const Triangle& t1, Triangle& t2) -> bool
    {
        return t1.area > t2.area;
    });

    for(size_t i=0; i<this->triangleList.size(); i++)
    {
        this->totalArea += this->triangleList[i].area;
    }



}

LightMesh::~LightMesh()
{
    delete this->randomGenerator;
}


bool LightMesh::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, 
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
        if(object->Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, backfaceCulling) && r.d < dist)
            return true;
    }

    return false;


}


glm::vec3  LightMesh::ComputeDiffuseSpecular(const Ray& ray, glm::vec3& diffuseReflectance, glm::vec3& specularReflectance,
                                             const float& phongExponent, const IntersectionReport& report,
                                             float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon,
                                             bool backfaceCulling, float time, std::vector<Object *>& objectPointerVector, bool degammaFlag, float gamma, bool hasBRDF, BRDF brdf, float refractiveIndex, float absorbtionIndex)
{

    SampleRandomPosition(ray);
    glm::vec3 result = glm::vec3(0.0);

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

    return result;   
}

void LightMesh::SampleRandomPosition(const Ray& ray)
{
    // We need to sample a direction to the light source first
    // We take square root and we want to push the value
    // closer to 1 because triangles are sorted according
    // to their areas and bigger area triangles should have
    // more probability to be selected.
    float randomValue = std::sqrt(this->randomGenerator->Generate());

    randomValue *= this->triangleList.size() - 1;
    int index = std::round(randomValue);
    Triangle selectedTriangle = this->triangleList[index];

    // we now sample a point on the selected triangle
    float tSampleRand1 = std::sqrt(this->randomGenerator->Generate());
    float tSampleRand2 = this->randomGenerator->Generate();

    glm::vec3 p = (1-tSampleRand2)*selectedTriangle.b + tSampleRand2*selectedTriangle.c;

    // the point we have found is in the local coordinates of the mesh
    glm::vec3 sampledPoint = tSampleRand1*p + (1-tSampleRand1)*selectedTriangle.a;

    // we transform sampledPoint to world coordinates
    glm::mat4 tMat = MotionBlurTranslate2(ray.time) * transformationMatrix;
    glm::vec3 worldPoint =(tMat * glm::vec4(sampledPoint, 1.0f));
    this->randomPosition = worldPoint;
}