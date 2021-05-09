#ifndef __SCENE_H__
#define __SCENE_H__

#include <iostream>
#include <RootDir.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tinyxml2.h>
#include <sstream>
#include <Utils.h>

#include <Sphere.h>
#include <Triangle.h>
#include <Mesh.h>
#include <MeshInstance.h>
#include <Object.h>

#include <omp.h>
#include <thread>
#include <atomic>
#include <Timer.h>
#include <future>
#include <iomanip>

#include <RandomGenerator.h>
#include <random>

struct WorkGroup
{
    int start;
    int end;
};

class Scene
{
private:

    bool backfaceCulling;

    RandomGenerator* randomVariableGenerator;
    RandomGenerator* cameraVariableGenerator;
    
    int _sampleNumber;
    std::vector<Sample> samples;

    std::vector<glm::mat4> _translationMatrices;
    std::vector<glm::mat4> _rotationMatrices;
    std::vector<glm::mat4> _scalingMatrices;

    tinyxml2::XMLNode* inputRoot;

    int worksize;
    int coreSize;
    std::atomic<int> count;
    std::vector<std::future<void>> futureVector;
    std::mutex progressLock;
    std::mutex mutex;
    std::condition_variable waitResults;

    std::stringstream stream;    

    glm::vec3               _backgroundColor;
    glm::vec3               _ambientLight; 

    std::vector<glm::vec3>     _vertexData;
    std::vector<glm::vec3>     _normalData;
    std::vector<int>           _neighborCount;

    std::vector<Triangle>     _triangles;
    std::vector<Sphere>       _spheres;
    std::vector<Mesh>         _meshes;
    std::vector<MeshInstance> _meshInstances;


    std::vector<Object*> _objectPointerVector;

    std::vector<PointLight> _pointLights;

    std::vector<Material>   _materials;
    std::vector<Camera>     _cameras;

    float _shadowRayEpsilon;
    float _intersectionTestEpsilon;    
    int _maxRecursionDepth;


    Camera _activeCamera; 
    std::vector<std::string> imageNames;
    float* _image;

    Ray ComputePrimaryRay(int i, int j);
    std::vector<RayWithWeigth> ComputePrimaryRays(int i, int j);
    void ClearImage();


    // RELATED TO RAY TRACING
    bool TestWorldIntersection(const Ray& ray,
                               IntersectionReport& report,
                               float tmin,
                               float tmax,
                               float intersectionTestEpsilon,
                               bool backfaceCulling);

    bool ShadowRayIntersection(
                               float tmin,
                               float tmax,
                               float intersectionTestEpsilon,
                               float shadowRayEpsilon,
                               const PointLight& light,
                               const IntersectionReport& report,
                               bool backfaceCulling);


    glm::vec3 ComputeAmbientComponent(const IntersectionReport& report);
    glm::vec3 ComputeDiffuseSpecular(const IntersectionReport& report, const Ray& ray);
    glm::vec3 ComputeSpecularComponent(const IntersectionReport& report, const PointLight& light, const Ray& ray);

    glm::vec3 RayTrace(const Ray& ray, bool backfaceCulling);
    glm::vec3 RecursiveTrace(const Ray& ray, const IntersectionReport& iR, int bounce, bool backfaceCulling);

    glm::vec3 TraceAndFilter(std::vector<RayWithWeigth> rwwVector);

    void RenderThread();

public:

    int _imageWidth;
    int _imageHeight;
    std::string _imageName;   
    Scene(const std::string& filepath);
    ~Scene();

    glm::vec2 GiveCoords(int index, int width);

    float* GetImage();
    void WritePixelCoord(int i, int j, const glm::vec3& color);
};


#endif /* __SCENE_H__ */