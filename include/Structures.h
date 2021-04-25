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
};

struct PointLight
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 intensity;
};

struct Material
{
    alignas(16) glm::vec3 ambientReflectance;
    alignas(16) glm::vec3 diffuseReflectance;
    alignas(16) glm::vec3 specularReflectance;
    alignas(16) glm::vec3 mirrorReflectance;
    alignas(16) glm::vec3 absorptionCoefficient;
    float phongExponent;
    int type; // 0: normal 1: mirror 2: dielectric 3: conductor
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

struct SplittedRays
{
    Ray reflected;
    Ray transmitted;
};

struct IntersectionReport
{
    alignas(16) glm::vec3 intersection;
    alignas(16) glm::vec3 normal;
    float d;    
    int materialId;

};



#endif