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
    SceneReadTextures(root, _images, _textures, _backgroundTextureIndex);    
    SceneReadLights(root, _pointLights, _areaLights, _environmentLights, _directionalLights, _spotLights, _images, _ambientLight);
    SceneReadMaterials(root, _materials);
    SceneReadVertexData(root, _vertexData);
    SceneReadTexCoordData(root, _texCoordData);
    SceneReadTransformations(root, _translationMatrices, _rotationMatrices, _scalingMatrices, _compositeMatrices);
    SceneReadMeshes(root, _meshes, _textures, _vertexData, _texCoordData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);
    SceneReadMeshInstances(root, _meshes, _textures, _meshInstances, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);
    SceneReadSpheres(root, _spheres, _textures, _vertexData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);
    SceneReadTriangles(root, _triangles, _textures, _vertexData, _texCoordData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);

    ScenePopulateObjects(_objectPointerVector, _meshes, _meshInstances, _spheres, _triangles);
    ScenePopulateLights(_lightPointerVector, _pointLights, _areaLights, _directionalLights, _spotLights, _environmentLights);

    _activeCamera = _cameras[0];

    _imageHeight = _activeCamera.imageResolution.y;
    _imageWidth  = _activeCamera.imageResolution.x;
    worksize = _imageHeight * _imageWidth;
    _image = new float[_imageHeight*_imageWidth*3];

    coreSize = std::thread::hardware_concurrency();
    count = 0;

    backfaceCulling = true;
    
    randomVariableGenerator = new RandomGenerator(0.0f, 1.0f);

    float halfAperture = _activeCamera.apertureSize/2;

    cameraVariableGenerator = new RandomGenerator(-halfAperture, halfAperture);

    areaLightPositionGenerator = new RandomGenerator(-0.5f, 0.5f);

    motionBlurTimeGenerator = new RandomGenerator(0.0f, 1.0f);

    glossyReflectionVarGenerator = new RandomGenerator(-0.5f, 0.5f);

    
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

    result.x = j;
    result.y = i;

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
                    
                    std::vector<RayWithWeigth> rwwVector = ComputePrimaryRays(coords.x, coords.y);
                    glm::vec3 filteredColor = TraceAndFilter(rwwVector, coords.x, coords.y);

                    //Ray pR = ComputePrimaryRay(coords.x, coords.y);
                    //glm::vec3 pixel = RayTrace(pR);
                    WritePixelCoord(coords.x, coords.y, filteredColor);

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
            _image[j * 3 + (_imageWidth * i *3)]     = 0.0;
            _image[j * 3 + (_imageWidth * i *3) + 1] = 0.0;
            _image[j * 3 + (_imageWidth * i *3) + 2] = 0.0;          
        }
    }    
}

void Scene::WritePixelCoord(int i, int j, const glm::vec3& color)
{
    _image[i * 3 + (_imageWidth * j *3)]     = color.x;
    _image[i * 3 + (_imageWidth * j *3) + 1] = color.y;
    _image[i * 3 + (_imageWidth * j *3) + 2] = color.z;
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


std::vector<RayWithWeigth> Scene::ComputePrimaryRays(int i, int j)
{
    std::vector<RayWithWeigth> result;
    int rowColSize = std::sqrt(_activeCamera.sampleNumber);

    for(int y=0; y<rowColSize; y++)
    {
        for(int x=0; x<rowColSize; x++)
        {
            float randomX = randomVariableGenerator->Generate();
            float randomY = randomVariableGenerator->Generate();

            glm::vec3 origin = _activeCamera.position;
            glm::vec3 m = origin + _activeCamera.gaze * _activeCamera.nearDistance;
            glm::vec3 q = m + _activeCamera.nearPlane.x * _activeCamera.v + _activeCamera.nearPlane.w * _activeCamera.up;

            float offsetX = (x + randomX)/rowColSize;
            float offsetY = (y + randomY)/rowColSize;

            if(rowColSize == 1)
            {
                offsetX = 0.5f;
                offsetY = 0.5f;
            }

            float su = (i + offsetX) * (_activeCamera.nearPlane.y - _activeCamera.nearPlane.x) / _activeCamera.imageResolution.x;
            float sv = (j + offsetY) * (_activeCamera.nearPlane.w -  _activeCamera.nearPlane.z) / _activeCamera.imageResolution.y;

            glm::vec3 direction = glm::normalize((q + su * _activeCamera.v - sv * _activeCamera.up) - origin);
            Ray fR(origin, direction);

            if(_activeCamera.apertureSize != 0)
            {

                float tfd = _activeCamera.focusDistance/glm::dot(-direction, -_activeCamera.gaze);
                glm::vec3 p = fR.origin + tfd * fR.direction;

                float apertureRandomOffset = cameraVariableGenerator->Generate();

                glm::vec3 s = origin;
                s.y += apertureRandomOffset;

                glm::vec3 bentDir = glm::normalize(p - s);

                fR = Ray(s, bentDir);
                
            }

            float time = motionBlurTimeGenerator->Generate();
            fR.time = time;

            RayWithWeigth rww;
            rww.r = fR;
            rww.distX = std::fabs(0.5f - offsetX);
            rww.distY = std::fabs(0.5f - offsetY);

            result.push_back(rww);
        }
    }
    
    return result;
}



bool Scene::TestWorldIntersection(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon, bool backfaceCulling)
{
    report.d = FLT_MAX;
    bool result = false;

    for(auto object : _objectPointerVector)
    {
        IntersectionReport r;
        if(object->Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, backfaceCulling))
        {
            result = true;
            report = r.d < report.d ? r : report;
        }
    }

    return result;
}

bool Scene::ShadowRayIntersection(float tmin, float tmax, float intersectionTestEpsilon, float shadowRayEpsilon, const glm::vec3& lightPosition, const IntersectionReport& report, bool backfaceCulling, float time)
{

    glm::vec3 direction = glm::normalize(lightPosition - report.intersection);
    glm::vec3 origin    = report.intersection + shadowRayEpsilon*report.normal;
    
    Ray ray(origin, direction);
    ray.time = time;

    float dist = glm::length(lightPosition - report.intersection);


    for(auto object : _objectPointerVector)
    {
        IntersectionReport r;
        if(object->Intersect(ray, r, tmin, tmax, intersectionTestEpsilon, backfaceCulling) && r.d < dist)
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

    if(report.diffuseActive && report.replaceAll)
        return report.texDiffuseReflectance;


    glm::vec3 diffuseReflectance  = _materials[report.materialId].diffuseReflectance;
    glm::vec3 specularReflectance = _materials[report.materialId].specularReflectance;
    float phongExponent           = _materials[report.materialId].phongExponent;

    for(size_t i=0; i<_lightPointerVector.size(); i++)
    {

        result += _lightPointerVector[i]->ComputeDiffuseSpecular(ray, diffuseReflectance, specularReflectance, phongExponent,
                                                                 report, 0.00001, 2000, _intersectionTestEpsilon, _shadowRayEpsilon, 
                                                                 true, ray.time, _objectPointerVector);

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


RayTraceResult Scene::RayTrace(const Ray& ray, bool backfaceCulling)
{

    RayTraceResult result;
    IntersectionReport r;
    if(TestWorldIntersection(ray, r, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
    {
        glm::vec3 pixel(0.0);        
        pixel += ComputeAmbientComponent(r) + ComputeDiffuseSpecular(r, ray) + RecursiveTrace(ray, r, 0, false);
        
        result.resultColor = pixel;
        result.hit = true;
        return result;
    }

    result.resultColor = glm::clamp(_backgroundColor, glm::vec3(0.0f), glm::vec3(FLT_MAX));
    result.hit = false;
    return result;

}

glm::vec3 Scene::TraceAndFilter(std::vector<RayWithWeigth> rwwVector, int x, int y)
{
    float stdDev = 1.f/6.f;
    glm::vec3 result;

    glm::vec3 weightedSum(0.f);
    glm::vec3 totalWeight(0.f);

    for(size_t i=0; i<rwwVector.size(); i++)
    {

        RayTraceResult rtResult = RayTrace(rwwVector[i].r, true);

        if(rtResult.hit)
        {
            weightedSum += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev) * rtResult.resultColor;
            totalWeight += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev);
        }
        else
        {
            if(_backgroundTextureIndex != -1)
            {
                float u = (float)x / (float)_imageWidth;
                float v = (float)y / (float)_imageHeight;

                glm::vec3 texColor = _textures[_backgroundTextureIndex]->Fetch(u, v);
                weightedSum += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev) * texColor;
                totalWeight += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev);
            }
            else if(_environmentLights.size() > 0)
            {
                glm::vec3 l = rwwVector[i].r.direction;
                glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
                glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
                glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

                float theta = std::acos(glm::dot(l,v));
                float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

                float tU = (-phi + M_PI) / (2 * M_PI);
                float tV = theta / M_PI;

                glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
                weightedSum += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev) * envColor;
                totalWeight += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev);                 
            }            
            else
            {            
                weightedSum += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev) * rtResult.resultColor;
                totalWeight += GaussianWeight(rwwVector[i].distX, rwwVector[i].distY, stdDev);                
            }
            
        }

    }

    result.x = weightedSum.x / totalWeight.x;
    result.y = weightedSum.y / totalWeight.y;
    result.z = weightedSum.z / totalWeight.z;

    return glm::clamp(result, glm::vec3(0.f), glm::vec3(FLT_MAX));
    
}


glm::vec3 Scene::RecursiveTrace(const Ray& ray, const IntersectionReport& iR, int bounce, bool backfaceCulling)
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

        if(_materials[iR.materialId].roughness > 0)
        {
            OrthonormalBasis basis = GiveOrthonormalBasis(reflectedRayDir);
            float randomOffsetU = glossyReflectionVarGenerator->Generate();
            float randomOffsetV = glossyReflectionVarGenerator->Generate();

            reflectedRayDir = reflectedRayDir + _materials[iR.materialId].roughness*(randomOffsetU*basis.u + randomOffsetV*basis.v);
        }

        Ray reflected(reflectedRayOrigin, reflectedRayDir);
        reflected.time = ray.time;
        reflected.isRefracting = ray.isRefracting;
        reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
        reflected.mediumCoeffNow    = ray.mediumCoeffNow;
        reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;

        IntersectionReport report;
        if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
        {
            result += attenuation * _materials[iR.materialId].mirrorReflectance * (ComputeAmbientComponent(report) + 
                                                                         ComputeDiffuseSpecular(report, reflected) +
                                                                         RecursiveTrace(reflected, report, bounce + 1, backfaceCulling));
        }
    }

    // Dielectric
    else if(_materials[iR.materialId].type == 1)
    {
        
        // Ray is entering
        if(glm::dot(ray.direction, iR.normal) < 0)
        {
            glm::vec3 reflectedRayOrigin = iR.intersection + iR.normal * 0.000001f;
            glm::vec3 reflectedRayDir    = glm::normalize(glm::reflect(ray.direction, iR.normal));

            float cosTheta = glm::dot(-ray.direction, iR.normal);
            float coeffRatio = 1/_materials[iR.materialId].refractionIndex;

            float cosPhiSquared = (1 - coeffRatio*coeffRatio * (1 - cosTheta*cosTheta));

            // Reflection and transmission both occur
            if(cosPhiSquared >= 0)
            {
                float cosPhi = std::sqrt(cosPhiSquared);

                // Building transmitted ray
                glm::vec3 transmittedRayOrigin = iR.intersection - iR.normal * 0.000001f;            
                glm::vec3 transmittedRayDir = (ray.direction + iR.normal*cosTheta)*coeffRatio - iR.normal*cosPhi;

                Ray tRay(transmittedRayOrigin, transmittedRayDir);
                tRay.mediumCoeffNow = _materials[iR.materialId].refractionIndex;
                tRay.mediumCoeffBefore = ray.mediumCoeffNow;
                tRay.isRefracting = true;
                tRay.materialIdCurrentlyIn = iR.materialId;
                tRay.time = ray.time;

                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.isRefracting = ray.isRefracting;
                reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                reflected.mediumCoeffNow = ray.mediumCoeffNow;
                reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;
                reflected.time = ray.time;

                float rRpar = (tRay.mediumCoeffNow*cosTheta - 1*cosPhi)/
                              (tRay.mediumCoeffNow*cosTheta + 1*cosPhi);

                float rPpar = (1*cosTheta - tRay.mediumCoeffNow*cosPhi)/
                              (1*cosTheta + tRay.mediumCoeffNow*cosPhi);

                float reflectionRatio = (rPpar*rPpar + rRpar*rRpar)/2;
                float transmissionRatio = 1 - reflectionRatio;

                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += reflectionRatio * (ComputeAmbientComponent(report) + ComputeDiffuseSpecular(report, reflected) + RecursiveTrace(reflected, report, bounce + 1, backfaceCulling));
                }
                IntersectionReport report2;
                if(TestWorldIntersection(tRay, report2, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += transmissionRatio * (RecursiveTrace(tRay, report2, bounce + 1, backfaceCulling));
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
                reflected.time = ray.time;

                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += (ComputeDiffuseSpecular(report, reflected) +
                                                               RecursiveTrace(reflected, report, bounce + 1, backfaceCulling));
                }                
            }            
        }

        
        // Ray is exiting
        else if(glm::dot(ray.direction, iR.normal) > 0)
        {
            glm::vec3 invertedNormal = -iR.normal;

            glm::vec3 reflectedRayOrigin = iR.intersection + invertedNormal * 0.000001f;
            glm::vec3 reflectedRayDir    = glm::reflect(ray.direction, invertedNormal);

            float cosTheta = glm::dot(-ray.direction, invertedNormal);
            float coeffRatio = ray.mediumCoeffNow/1;

            float cosPhiSquared = (1 - coeffRatio*coeffRatio * (1 - cosTheta*cosTheta));

            // Reflection and transmission both occur
            if(cosPhiSquared >= 0)
            {
                float cosPhi = std::sqrt(cosPhiSquared);

                // Building transmitted ray
                glm::vec3 transmittedRayOrigin = iR.intersection - invertedNormal * 0.000001f;            
                glm::vec3 transmittedRayDir = (ray.direction + invertedNormal*cosTheta)*coeffRatio - invertedNormal*cosPhi;

                Ray tRay(transmittedRayOrigin, transmittedRayDir);
                tRay.mediumCoeffBefore = ray.mediumCoeffNow;
                tRay.mediumCoeffNow = 1;
                tRay.isRefracting = false;
                tRay.materialIdCurrentlyIn = -1;
                tRay.time = ray.time;

                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.isRefracting = ray.isRefracting;
                reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                reflected.mediumCoeffNow = ray.mediumCoeffNow;
                reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;
                reflected.time = ray.time;

                float rRpar = (1*cosTheta - tRay.mediumCoeffBefore*cosPhi)/
                              (1*cosTheta + tRay.mediumCoeffBefore*cosPhi);

                float rPpar = (tRay.mediumCoeffBefore*cosTheta - 1*cosPhi)/
                              (tRay.mediumCoeffBefore*cosTheta + 1*cosPhi);

                float reflectionRatio = (rPpar*rPpar + rRpar*rRpar)/2;
                float transmissionRatio = 1 - reflectionRatio;


                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += reflectionRatio * attenuation * (RecursiveTrace(reflected, report, bounce + 1, backfaceCulling));
                }
                IntersectionReport report2;
                if(TestWorldIntersection(tRay, report2, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += transmissionRatio * attenuation * (ComputeAmbientComponent(report2) + ComputeDiffuseSpecular(report2, tRay) + RecursiveTrace(tRay, report2, bounce + 1, backfaceCulling));
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
                reflected.time = ray.time;

                IntersectionReport report;
                if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += attenuation * (RecursiveTrace(reflected, report, bounce + 1, backfaceCulling));
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

        OrthonormalBasis basis = GiveOrthonormalBasis(reflectedRayDir);
        float randomOffsetU = glossyReflectionVarGenerator->Generate();
        float randomOffsetV = glossyReflectionVarGenerator->Generate();

        reflectedRayDir = reflectedRayDir + _materials[iR.materialId].roughness*(randomOffsetU*basis.u + randomOffsetV*basis.v);
        
        Ray reflected(reflectedRayOrigin, reflectedRayDir);
        reflected.isRefracting = ray.isRefracting;
        reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
        reflected.mediumCoeffNow    = ray.mediumCoeffNow;
        reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;
        reflected.time = ray.time;

        IntersectionReport report;
        if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
        {
            result += attenuation * reflectionRatio * _materials[iR.materialId].mirrorReflectance * (
                                                                         ComputeDiffuseSpecular(report, reflected) +
                                                                         RecursiveTrace(reflected, report, bounce + 1, backfaceCulling));
        }        
    }

    return result;
}