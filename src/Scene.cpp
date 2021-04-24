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
        glm::vec3 pixel(0.5, 0.0, 0.7);
        WritePixelCoord(coords.x, coords.y, pixel);
    }
}

float* Scene::GetImage()
{

    PopulateWorkGroups();

    double start = omp_get_wtime();
    #pragma omp parallel for num_threads(16)
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
