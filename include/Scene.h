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
#include <omp.h>

struct WorkGroup
{
    int start;
    int end;
};

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


    Camera _activeCamera; 
    std::vector<std::string> imageNames;
    float* _image;

    std::vector<Ray> _primaryRayPool;

    void PopulateWorkGroups();
    void ProcessWorkGroup(WorkGroup wg);

    Ray ComputePrimaryRay(int i, int j);

    void ClearImage();

    int _workGroupSize = 16;

    std::vector<WorkGroup> _workGroups;

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