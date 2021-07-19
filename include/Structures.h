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
    PHOTOGRAPHIC = 0
};

enum RenderMode
{
    CLASSIC = 0,
    HDR     = 1
};

enum BRDFType
{
    ORIGINAL_PHONG      = 0,
    MODIFIED_PHONG       = 1,
    ORIGINAL_BLINN_PHONG = 2,
    MODIFIED_BLINN_PHONG = 3,
    TORRANCE_SPARROW     = 4
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
    TMO tmo = TMO::PHOTOGRAPHIC;
    float keyValue = 0.18;
    float burn_percentage = 1.0;
    float saturation = 1.0;
    float gamma = 2.2;

    std::string imageName;


};

struct BRDF 
{
    BRDFType type;
    float exponent;
    bool normalized;
    bool kdfresnel;
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

    bool degammaFlag;
    bool hasBrdf;
    BRDF brdf;
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
    bool isLight = false;

    float throughput;

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