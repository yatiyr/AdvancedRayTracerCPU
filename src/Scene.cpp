#include <Scene.h>


Scene::Scene(const std::string& filepath)
{
    tinyxml2::XMLDocument file;
    std::stringstream stream;

    auto res = file.LoadFile(filepath.c_str());
    if(res)
    {
        throw std::runtime_error("Error: The xml file cannot be loaded");
    }

    auto root = file.FirstChild();
    if(!root)
    {
        throw std::runtime_error("Error: Root is not found.");
    }


    SceneReadConstants(root, _backgroundColor, _shadowRayEpsilon, _intersectionTestEpsilon, _maxRecursionDepth);
    SceneReadCameras(root, _cameras, imageNames, _imageName);
    SceneReadLights(root, _pointLights, _ambientLight);
    SceneReadMaterials(root, _materials);
    SceneReadVertexData(root, _vertexData);
    SceneReadMeshes(root, _meshes, _vertexData);
    SceneReadSpheres(root, _spheres, _vertexData);
    SceneReadTriangles(root, _triangles, _vertexData);

    _activeCamera = _cameras[0];

    _imageHeight = _activeCamera.imageResolution.y;
    _imageWidth  = _activeCamera.imageResolution.x;

    _image = new float[_imageHeight*_imageWidth*4];
}

Scene::~Scene()
{
    delete[] _image;
}

glm::vec2 Scene::GiveCoords(int index, int width)
{
    glm::vec2 result;

    int i = index/width;
    int j = index%width;

    result.x = i;
    result.y = j;

    return result;
}

void Scene::PopulateWorkGroups()
{
    int totalSize = _imageHeight * _imageWidth;
    int chunkSize = totalSize / _workGroupSize;

    for(int i=0; i<_workGroupSize; i++)
    {
        int start;
        int end;

        start = i * chunkSize;
        if(i == _workGroupSize - 1)
            end = (i+1) * chunkSize - 1 + (totalSize%chunkSize);
        else
            end = (i+1) * chunkSize - 1;

        WorkGroup wg;
        wg.start = start;
        wg.end   = end;
        _workGroups.push_back(wg);
    }
}

void Scene::ProcessWorkGroup(WorkGroup wg)
{
    for(int i=wg.start; i<=wg.end; i++)
    {
        glm::vec2 coords = GiveCoords(i, _imageWidth);
        Ray pR = ComputePrimaryRay(coords.x, coords.y);
        glm::vec3 pixel = RayTrace(pR);
        WritePixelCoord(coords.x, coords.y, pixel);
    }
}

float* Scene::GetImage()
{

    PopulateWorkGroups();

    double start = omp_get_wtime();
    #pragma omp parallel for num_threads(1)
    for(int i=0; i<_workGroupSize; i++)
    {
        ProcessWorkGroup(_workGroups[i]);
    }
    double end = omp_get_wtime();

    std::cout << "Image rendered in --->" << end - start << " s.\n";

    return _image;
}

void Scene::ClearImage()
{
    for(int i=0; i<_imageHeight; i++)
    {
        for(int j=0; j<_imageWidth; j++)
        {
            _image[j * 4 + (_imageWidth * i *4)]     = 0.0;
            _image[j * 4 + (_imageWidth * i *4) + 1] = 0.0;
            _image[j * 4 + (_imageWidth * i *4) + 2] = 0.0;
            _image[j * 4 + (_imageWidth * i *4) + 3] = 1.0f;            
        }
    }    
}

void Scene::WritePixelCoord(int i, int j, const glm::vec3& color)
{
    _image[j * 4 + (_imageWidth * i *4)]     = color.x;
    _image[j * 4 + (_imageWidth * i *4) + 1] = color.y;
    _image[j * 4 + (_imageWidth * i *4) + 2] = color.z;
    _image[j * 4 + (_imageWidth * i *4) + 3] = 1.0f;
}


Ray Scene::ComputePrimaryRay(int i, int j)
{
    Ray r;
    r.origin = _activeCamera.position;
    glm::vec3 m = r.origin + _activeCamera.gaze * _activeCamera.nearDistance;
    glm::vec3 q = m + _activeCamera.nearPlane.x * _activeCamera.v + _activeCamera.nearPlane.w * _activeCamera.up;

    float su = (j + 0.5) * (_activeCamera.nearPlane.y - _activeCamera.nearPlane.x) / _activeCamera.imageResolution.x;
    float sv = (i + 0.5) * (_activeCamera.nearPlane.w - _activeCamera.nearPlane.z) / _activeCamera.imageResolution.y;

    r.direction = glm::normalize((q + su*_activeCamera.v - sv*_activeCamera.up) - r.origin);

    return r;    
}


bool Scene::TestWorldIntersection(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon)
{
    report.d = FLT_MAX;
    bool result = false;

    for(size_t i=0; i< _meshes.size(); i++)
    {
        IntersectionReport r;
        if(ray.direction.x == -0.386324555f && ray.direction.y == -0.386324555f && ray.direction.z == -0.837559998f)
            int eren = 0;
        if(_meshes[i].Intersect(ray, r, tmin, tmax, intersectionTestEpsilon))
        {
            result = true;
            report = r.d < report.d ? r : report;
        }
    }

    for(size_t i=0; i < _triangles.size(); i++)
    {
        IntersectionReport r;
        if(_triangles[i].Intersect(ray, r, tmin, tmax, intersectionTestEpsilon))
        {
            result = true;
            report = r.d < report.d ? r : report;
        }
    }

    for(size_t i=0; i<_spheres.size(); i++)
    {
        IntersectionReport r;
        if(_spheres[i].Intersect(ray, r, tmin, tmax))
        {
            result = true;
            report = r.d < report.d ? r : report;
        }
    }


    return result;
}

bool Scene::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, const PointLight& light, const IntersectionReport& report)
{

    Ray ray;

    ray.direction = glm::normalize(light.position - report.intersection);
    ray.origin    = report.intersection + shadowRayEpsilon*report.normal;

    float dist = glm::length(light.position - report.intersection);


    for(size_t i=0; i<_meshes.size(); i++)
    {
        IntersectionReport r;
        if(_meshes[i].Intersect(ray, r, tmin, tmax, intersectionTestEpsilon) && r.d < dist)
            return true;

    }


    for(size_t i=0; i<_triangles.size(); i++)
    {
        IntersectionReport r;
        if(_triangles[i].Intersect(ray, r, tmin, tmax, intersectionTestEpsilon) && r.d < dist)
            return true;
    }

    for(size_t i=0; i<_spheres.size(); i++)
    {
        IntersectionReport r;
        if(_spheres[i].Intersect(ray, r, tmin, tmax) && r.d < dist)
            return true;
    }

    return false;
}


glm::vec3 Scene::ComputeAmbientComponent(const IntersectionReport& report)
{
    int materialId = report.materialId;
    if(materialId != 0 && materialId != 1 && materialId != 2 && materialId != 3 && materialId != 4 && materialId != 5 )
        int eren = 0;
    return _ambientLight * _materials[report.materialId].ambientReflectance;
}

glm::vec3 Scene::ComputeDiffuseComponent(const IntersectionReport& report, const PointLight& light)
{
    glm::vec3 result = glm::vec3(0.0);

    float lightDistance = glm::length(light.position - report.intersection);
    glm::vec3 wi = glm::normalize(light.position - report.intersection);
    result += _materials[report.materialId].diffuseReflectance *
              std::max(0.0f, glm::dot(wi, report.normal)) *
              (light.intensity / (lightDistance * lightDistance));
    return result;
}

glm::vec3 Scene::ComputeSpecularComponent(const IntersectionReport& report, const PointLight& light, const Ray& ray)
{
    glm::vec3 result = glm::vec3(0.0);

    float lightDistance = glm::length(light.position - report.intersection);
    glm::vec3 wi = glm::normalize(light.position - report.intersection);
    glm::vec3 h  = glm::normalize(wi - ray.direction);

    result += _materials[report.materialId].specularReflectance *
              std::pow(std::max(0.0f, glm::dot(report.normal, h)), _materials[report.materialId].phongExponent) *
              (light.intensity / (lightDistance * lightDistance));

    return result;
}


glm::vec3 Scene::RayTrace(const Ray& ray)
{
    glm::vec3 pixel(0.0);
    IntersectionReport r;

    if(TestWorldIntersection(ray, r, 0, 2000, _intersectionTestEpsilon))
    {
        pixel += ComputeAmbientComponent(r);
        
        for(size_t i=0; i<_pointLights.size(); i++)
        {
            if(ShadowRayIntersection(0, 2000, _intersectionTestEpsilon, _shadowRayEpsilon, _pointLights[i], r))
            {
                continue;
            }
            else
            {
                pixel += ComputeDiffuseComponent(r, _pointLights[i]) + ComputeSpecularComponent(r, _pointLights[i], ray);
            }
        }
    }

    return glm::clamp(pixel, glm::vec3(0.0f), glm::vec3(1.0f));


}