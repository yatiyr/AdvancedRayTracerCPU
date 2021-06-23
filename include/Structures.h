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
#include <Texture.h>

enum TMO
{
    PHOTOHRAPHIC = 0
};

enum RenderMode
{
    CLASSIC = 0,
    HDR     = 1
};

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


    // Rendering Mode
    RenderMode renderMode = RenderMode::CLASSIC;    

    // Tone Map values  
    TMO tmo = TMO::PHOTOHRAPHIC;
    float keyValue = 0.18;
    float burn_percentage = 1.0;
    float saturation = 1.0;
    float gamma = 2.2;


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

struct DirectionalLight
{
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 radiance;
};

struct SphericalDirectionalLight
{
    Texture hdrTexture;
};

struct SpotLight
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 intensity;

    // Angles will be converted to radians
    // while creating the struct
    float coverageAngle;
    float falloffAngle;
    float exponent = 4;


    float GetFollowFactor(float theta)
    {
        return std::pow((theta - std::cos(coverageAngle/2))/
                        (std::cos(falloffAngle/2) - std::cos(coverageAngle/2)),exponent);
    }

    glm::vec3 GetL(glm::vec3 position)
    {

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
    float roughness;
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

    // Normal or bump map can change normal
    alignas(16) glm::vec3 normal;

    alignas(16) glm::vec3 texDiffuseReflectance;
    alignas(16) glm::vec3 texSpecularReflectance;

    int materialId;

    float d;
    float texRoughness;

    bool hasTexture;

    // If true, diffuseReflectance becomes final color
    bool replaceAll;

    // 1: Replace KD
    // 2: Blend KD
    int texDiffuseKdMode;
    int texSpecularKdMode;

    uint8_t enabledTextures;

    glm::vec2 texCoord;

    glm::vec3 coordA;
    glm::vec3 coordB;
    glm::vec3 coordC;

    glm::vec2 texCoordA;
    glm::vec2 texCoordB;
    glm::vec2 texCoordC;

    bool diffuseActive = false;
    bool specularActive = false;
    bool emissionActive = false;

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

struct OrthonormalBasis
{
    alignas(16) glm::vec3 u;
    alignas(16) glm::vec3 v;
};

#endif