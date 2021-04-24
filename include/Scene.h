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


class Scene
{
private:

    tinyxml2::XMLNode* inputRoot;
    std::stringstream stream;    

    glm::vec3               _backgroundColor;
    glm::vec3               _ambientLight; 

    std::vector<glm::vec3>     _vertexData;
    std::vector<Triangle>   _triangles;
    std::vector<Sphere>     _spheres;
    std::vector<Mesh>       _meshes;

    std::vector<PointLight> _pointLights;

    std::vector<Material>   _materials;
    std::vector<Camera>     _cameras;

    float _shadowRayEpsilon;
    float _intersectionTestEpsilon;    
    int _maxRecursionDepth;

    Camera                 _activeCamera;
    std::vector<std::string> imageNames;
    std::string _imageName;

    void ReadConstants();
    void ReadCameras();
    void ReadLights();
    void ReadMaterials();
    void ReadVertexData();
    void ReadObjects();

    void ReadMeshes();
    void ReadSpheres();
    void ReadTriangles();



public:

    Scene(const std::string& filepath);
    ~Scene();

    float* GetImage();
};


#endif /* __SCENE_H__ */