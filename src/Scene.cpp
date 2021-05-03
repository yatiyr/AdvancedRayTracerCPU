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
    worksize = _imageHeight * _imageWidth;
    _image = new float[_imageHeight*_imageWidth*4];

    coreSize = std::thread::hardware_concurrency();
    count = 0;

    
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

void Scene::RenderThread()
{
    int cores = coreSize;

    while(cores--)
    {
        futureVector.push_back(
            std::async(std::launch::async, [=]()
            {
                while(true)
                {
                    int index = count++;
                    if(index >= worksize)
                        break;

                    glm::vec2 coords = GiveCoords(index, _imageWidth);
                    Ray pR = ComputePrimaryRay(coords.x, coords.y);
                    glm::vec3 pixel = RayTrace(pR);
                    WritePixelCoord(coords.x, coords.y, pixel);

/*
                    {
                        std::lock_guard<std::mutex> lock(progressLock);
                        std::cout << "Progress: [" << std::setprecision(1) << std::fixed << (count / (float)worksize) * 100.0 << "% ] \r";
                        std::cout.flush();
                    } */
                }
            }));
    }


    for(auto& element : futureVector)
    {
        element.get();
    }
}

float* Scene::GetImage()
{
    Timer t;
    RenderThread();
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
    glm::vec3 origin = _activeCamera.position;
    glm::vec3 m = origin + _activeCamera.gaze * _activeCamera.nearDistance;
    glm::vec3 q = m + _activeCamera.nearPlane.x * _activeCamera.v + _activeCamera.nearPlane.w * _activeCamera.up;

    float su = (j + 0.5) * (_activeCamera.nearPlane.y - _activeCamera.nearPlane.x) / _activeCamera.imageResolution.x;
    float sv = (i + 0.5) * (_activeCamera.nearPlane.w - _activeCamera.nearPlane.z) / _activeCamera.imageResolution.y;

    glm::vec3 direction = glm::normalize((q + su*_activeCamera.v - sv*_activeCamera.up) - origin);


    return Ray(origin, direction);    
}


bool Scene::TestWorldIntersection(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon)
{
    report.d = FLT_MAX;
    bool result = false;

    for(size_t i=0; i< _meshes.size(); i++)
    {
        IntersectionReport r;
        if(_meshes[i].Intersect(ray, r, tmin, tmax, intersectionTestEpsilon))
        {
            result = true;
            report = r.d < report.d ? r : report;
        }
    }

    for(size_t i=0; i < _triangles.size(); i++)
    {
        IntersectionReport r;
        if(_triangles[i].Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, false))
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

    glm::vec3 direction = glm::normalize(light.position - report.intersection);
    glm::vec3 origin    = report.intersection + shadowRayEpsilon*report.normal;
    
    Ray ray(origin, direction);

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
        if(_triangles[i].Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, false) && r.d < dist)
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
    return _ambientLight * _materials[report.materialId].ambientReflectance;
}

glm::vec3 Scene::ComputeDiffuseSpecular(const IntersectionReport& report, const Ray& ray)
{
    glm::vec3 result = glm::vec3(0.0);

    for(size_t i=0; i<_pointLights.size(); i++)
    {
        if(ShadowRayIntersection(0, 2000, _intersectionTestEpsilon, _shadowRayEpsilon, _pointLights[i], report))
        {
            continue;
        }
        else
        {
            float lightDistance = glm::length(_pointLights[i].position - report.intersection);
            glm::vec3 wi = glm::normalize(_pointLights[i].position - report.intersection);

            // Diffuse Calculation
            result += _materials[report.materialId].diffuseReflectance *
                    std::max(0.0f, glm::dot(wi, report.normal)) *
                    (_pointLights[i].intensity / (lightDistance * lightDistance));


            // Specular Calculation
            glm::vec3 h  = glm::normalize(wi - ray.direction);

            result += _materials[report.materialId].specularReflectance *
                    std::pow(std::max(0.0f, glm::dot(report.normal, h)), _materials[report.materialId].phongExponent) *
                    (_pointLights[i].intensity / (lightDistance * lightDistance));
        }
    }

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

    IntersectionReport r;
    if(TestWorldIntersection(ray, r, 0, 2000, _intersectionTestEpsilon))
    {
        glm::vec3 pixel(0.0);        
        pixel += ComputeAmbientComponent(r) + ComputeDiffuseSpecular(r, ray) + RecursiveTrace(ray, r, 0);
        

        return glm::clamp(pixel, glm::vec3(0.0f), glm::vec3(1.0f));
    }

    return glm::clamp(_backgroundColor, glm::vec3(0.0f), glm::vec3(1.0f));

}


glm::vec3 Scene::RecursiveTrace(const Ray& ray, const IntersectionReport& iR, int bounce)
{

    glm::vec3 result(0.0);

    if(bounce >= _maxRecursionDepth)
        return result;

    // Diffuse element
    if(_materials[iR.materialId].type == -1)
        return result;

    glm::vec3 attenuation(1.0);

    if(ray.materialIdCurrentlyIn != -1)
    {
        float dist = glm::length(ray.origin - iR.intersection);
        attenuation.x = std::pow((float)EULER, -_materials[ray.materialIdCurrentlyIn].absorptionCoefficient.x *dist);
        attenuation.y = std::pow((float)EULER, -_materials[ray.materialIdCurrentlyIn].absorptionCoefficient.y *dist);
        attenuation.z = std::pow((float)EULER, -_materials[ray.materialIdCurrentlyIn].absorptionCoefficient.z *dist);                                
    }

    // Mirror
    if(_materials[iR.materialId].type == 0)
    {
        glm::vec3 reflectedRayOrigin = iR.intersection + iR.normal*_shadowRayEpsilon;
        glm::vec3 reflectedRayDir    = glm::normalize(glm::reflect(ray.direction, iR.normal));

        Ray reflected(reflectedRayOrigin, reflectedRayDir);
        reflected.isRefracting = ray.isRefracting;
        reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
        reflected.mediumCoeffNow    = ray.mediumCoeffNow;
        reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;

        IntersectionReport report;
        if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon))
        {
            result += attenuation * _materials[iR.materialId].mirrorReflectance * (ComputeAmbientComponent(report) + 
                                                                         ComputeDiffuseSpecular(report, reflected) +
                                                                         RecursiveTrace(reflected, report, bounce + 1));
        }
    }

    // Dielectric
    else if(_materials[iR.materialId].type == 1)
    {
        
        // Ray is entering
        if(glm::dot(ray.direction, iR.normal) < 0)
        {
            glm::vec3 reflectedRayOrigin = iR.intersection + iR.normal * _shadowRayEpsilon;
            glm::vec3 reflectedRayDir    = glm::normalize(glm::reflect(ray.direction, iR.normal));

            float cosTheta = glm::dot(-ray.direction, iR.normal);
            float coeffRatio = 1/_materials[iR.materialId].refractionIndex;

            float cosPhiSquared = (1 - coeffRatio*coeffRatio * (1 - cosTheta*cosTheta));

            // Reflection and transmission both occur
            if(cosPhiSquared >= 0)
            {
                float cosPhi = std::sqrt(cosPhiSquared);

                // Building transmitted ray
                glm::vec3 transmittedRayOrigin = iR.intersection - iR.normal * _shadowRayEpsilon;            
                glm::vec3 transmittedRayDir = (ray.direction + iR.normal*cosTheta)*coeffRatio - iR.normal*cosPhi;

                Ray tRay(transmittedRayOrigin, transmittedRayDir);
                tRay.mediumCoeffNow = _materials[iR.materialId].refractionIndex;
                tRay.mediumCoeffBefore = ray.mediumCoeffNow;
                tRay.isRefracting = true;
                tRay.materialIdCurrentlyIn = iR.materialId;

                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.isRefracting = ray.isRefracting;
                reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                reflected.mediumCoeffNow = ray.mediumCoeffNow;
                reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;

                float rRpar = (tRay.mediumCoeffNow*cosTheta - 1*cosPhi)/
                              (tRay.mediumCoeffNow*cosTheta + 1*cosPhi);

                float rPpar = (1*cosTheta - tRay.mediumCoeffNow*cosPhi)/
                              (1*cosTheta + tRay.mediumCoeffNow*cosPhi);

                float reflectionRatio = (rPpar*rPpar + rRpar*rRpar)/2;
                float transmissionRatio = 1 - reflectionRatio;

                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon))
                {
                    result += reflectionRatio * attenuation * (ComputeAmbientComponent(report) + ComputeDiffuseSpecular(report, reflected) + RecursiveTrace(reflected, report, bounce + 1));
                }
                IntersectionReport report2;
                if(TestWorldIntersection(tRay, report2, 0, 2000, _intersectionTestEpsilon))
                {
                    result += transmissionRatio * attenuation * (RecursiveTrace(tRay, report2, bounce + 1));
                }

            }
            // Only reflection occurs
            else
            {
                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.isRefracting = ray.isRefracting;
                reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                reflected.mediumCoeffNow = ray.mediumCoeffNow;
                reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;

                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon))
                {
                    result += attenuation * (ComputeDiffuseSpecular(report, reflected) +
                                                               RecursiveTrace(reflected, report, bounce + 1));
                }                
            }            
        }

        
        // Ray is exiting
        else if(glm::dot(ray.direction, iR.normal) > 0)
        {
            glm::vec3 invertedNormal = -iR.normal;

            glm::vec3 reflectedRayOrigin = iR.intersection + invertedNormal * _shadowRayEpsilon;
            glm::vec3 reflectedRayDir    = glm::reflect(ray.direction, invertedNormal);

            float cosTheta = glm::dot(-ray.direction, invertedNormal);
            float coeffRatio = ray.mediumCoeffNow/1;

            float cosPhiSquared = (1 - coeffRatio*coeffRatio * (1 - cosTheta*cosTheta));

            // Reflection and transmission both occur
            if(cosPhiSquared >= 0)
            {
                float cosPhi = std::sqrt(cosPhiSquared);

                // Building transmitted ray
                glm::vec3 transmittedRayOrigin = iR.intersection - invertedNormal * _shadowRayEpsilon;            
                glm::vec3 transmittedRayDir = (ray.direction + invertedNormal*cosTheta)*coeffRatio - invertedNormal*cosPhi;

                Ray tRay(transmittedRayOrigin, transmittedRayDir);
                tRay.mediumCoeffBefore = ray.mediumCoeffNow;
                tRay.mediumCoeffNow = 1;
                tRay.isRefracting = false;
                tRay.materialIdCurrentlyIn = -1;

                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.isRefracting = ray.isRefracting;
                reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                reflected.mediumCoeffNow = ray.mediumCoeffNow;
                reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;

                float rRpar = (1*cosTheta - tRay.mediumCoeffBefore*cosPhi)/
                              (1*cosTheta + tRay.mediumCoeffBefore*cosPhi);

                float rPpar = (tRay.mediumCoeffBefore*cosTheta - 1*cosPhi)/
                              (tRay.mediumCoeffBefore*cosTheta + 1*cosPhi);

                float reflectionRatio = (rPpar*rPpar + rRpar*rRpar)/2;
                float transmissionRatio = 1 - reflectionRatio;


                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon))
                {
                    result += reflectionRatio * attenuation * (RecursiveTrace(reflected, report, bounce + 1));
                }
                IntersectionReport report2;
                if(TestWorldIntersection(tRay, report2, 0, 2000, _intersectionTestEpsilon))
                {
                    result += transmissionRatio * attenuation * (ComputeAmbientComponent(report2) + ComputeDiffuseSpecular(report2, tRay) + RecursiveTrace(tRay, report2, bounce + 1));
                }

            }
            // Only reflection occurs
            else
            {
                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.isRefracting = ray.isRefracting;
                reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                reflected.mediumCoeffNow = ray.mediumCoeffNow;
                reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;

                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon))
                {
                    result += attenuation * (RecursiveTrace(reflected, report, bounce + 1));
                }                
            } 
        } 

    }

    // Conductor
    else if(_materials[iR.materialId].type == 2)
    {
        glm::vec3 reflectedRayOrigin = iR.intersection + iR.normal*_shadowRayEpsilon;
        glm::vec3 reflectedRayDir    = glm::normalize(glm::reflect(ray.direction, iR.normal));
        float cosTheta = glm::dot(-ray.direction, iR.normal);
        float aI = _materials[iR.materialId].absorptionIndex;
        float rI = _materials[iR.materialId].refractionIndex;

        float rS = ((rI*rI + aI*aI) - 2*rI*cosTheta + (cosTheta*cosTheta))/
                   ((rI*rI + aI*aI) + 2*rI*cosTheta + (cosTheta*cosTheta));

        float rP = ((rI*rI + aI*aI)*(cosTheta*cosTheta) - 2*rI*cosTheta + 1)/
                   ((rI*rI + aI*aI)*(cosTheta*cosTheta) + 2*rI*cosTheta + 1);

        float reflectionRatio = (rS + rP)/2;
        
        Ray reflected(reflectedRayOrigin, reflectedRayDir);
        reflected.isRefracting = ray.isRefracting;
        reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
        reflected.mediumCoeffNow    = ray.mediumCoeffNow;
        reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;

        IntersectionReport report;
        if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon))
        {
            result += attenuation * reflectionRatio * _materials[iR.materialId].mirrorReflectance * (
                                                                         ComputeDiffuseSpecular(report, reflected) +
                                                                         RecursiveTrace(reflected, report, bounce + 1));
        }        
    }

    return result;
}