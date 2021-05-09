#ifndef __STRUCTURES_H__
#define __STRUCTURES_H__

#include <glm/glm.hpp>
#include <set>
#include <vector>
#include <array>
#include <algorithm>
#include <list>
#include <stack>
#include <Ray.h>

struct Camera
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 gaze;
    alignas(16) glm::vec3 up;
    alignas(16) glm::vec3 v;
    alignas(16) glm::vec4 nearPlane;
    glm::vec2 imageResolution;       
    float nearDistance;
    float focusDistance;
    float apertureSize;
    int sampleNumber;
};

struct PointLight
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 intensity;
};

struct AreaLight
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 radiance;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec3 u;
    alignas(16) glm::vec3 v;
    float extent;

    AreaLight(glm::vec3 position, glm::vec3 radiance, glm::vec3 normal, float extent)
    {
        this->normal = normal;
        this->position = position;
        this->radiance = radiance;
        this->extent = extent;

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

    }
};

struct Material
{
    alignas(16) glm::vec3 ambientReflectance;
    alignas(16) glm::vec3 diffuseReflectance;
    alignas(16) glm::vec3 specularReflectance;
    alignas(16) glm::vec3 mirrorReflectance;
    alignas(16) glm::vec3 absorptionCoefficient;
    float phongExponent;
    int type;
    float refractionIndex;
    float absorptionIndex;
};

struct Vertex
{
    alignas(16) glm::vec3 pos;
};

struct Indices
{
    int a;
    int b;
    int c;
};

struct IntersectionReport
{
    alignas(16) glm::vec3 intersection;
    alignas(16) glm::vec3 normal;
    float d;    
    int materialId;

};


struct Sample
{
    float x;
    float y;
};

struct RayWithWeigth
{
    Ray r;
    float distX;
    float distY;
};


#endif