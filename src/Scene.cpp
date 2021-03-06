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
    SceneReadBRDF(root, _brdfs);
    SceneReadMaterials(root, _materials, _brdfs);
    SceneReadVertexData(root, _vertexData);
    SceneReadTexCoordData(root, _texCoordData);
    SceneReadTransformations(root, _translationMatrices, _rotationMatrices, _scalingMatrices, _compositeMatrices);
    SceneReadMeshes(root, _meshes, _textures, _vertexData, _texCoordData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);
    SceneReadMeshInstances(root, _meshes, _textures, _meshInstances, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);
    SceneReadSpheres(root, _spheres, _textures, _vertexData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);
    SceneReadTriangles(root, _triangles, _textures, _vertexData, _texCoordData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);

    SceneReadLightMeshes(root, _lightMeshes, _textures, _vertexData, _texCoordData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);
    SceneReadLightSpheres(root, _lightSpheres, _textures, _vertexData, _rotationMatrices, _scalingMatrices, _translationMatrices, _compositeMatrices);    


    ScenePopulateObjects(_objectPointerVector, _lightObjectPointerVector ,_meshes, _meshInstances, _spheres, _triangles, _lightMeshes, _lightSpheres);
    ScenePopulateLights(_lightPointerVector, _pointLights, _areaLights, _directionalLights, _spotLights, _environmentLights, _lightMeshes, _lightSpheres);

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

    directionSampler = new DirectionSampler();

    
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

                    {
                        std::lock_guard<std::mutex> lock(progressLock);
                        std::cout << "Progress: [" << std::setprecision(1) << std::fixed << (count / (float)worksize) * 100.0 << "% ] \r";
                        std::cout.flush();
                    }
                }
            }));
    }


    for(auto& element : futureVector)
    {
        element.get();
    }

    futureVector.clear();
    count = 0;
}

float* Scene::GetImage()
{
    Timer t;
    RenderThread();
    std::cout << "Rendered: " << _activeCamera.imageName << " ";
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
    bool gammaflag = _materials[report.materialId].degammaFlag;

    if(!gammaflag)
        return _ambientLight * _materials[report.materialId].ambientReflectance;
    else
    {
        glm::vec3 aR = _materials[report.materialId].ambientReflectance;

        aR.x = std::pow(_materials[report.materialId].ambientReflectance.x,_activeCamera.gamma);
        aR.y = std::pow(_materials[report.materialId].ambientReflectance.y,_activeCamera.gamma);
        aR.z = std::pow(_materials[report.materialId].ambientReflectance.z,_activeCamera.gamma);

        return _ambientLight * aR;        
       
    }

}

glm::vec3 Scene::ComputeDiffuseSpecular(const IntersectionReport& report, const Ray& ray)
{
    glm::vec3 result = glm::vec3(0.0);

    if(report.diffuseActive && report.replaceAll)
        return report.texDiffuseReflectance;


    glm::vec3 diffuseReflectance  = _materials[report.materialId].diffuseReflectance;
    glm::vec3 specularReflectance = _materials[report.materialId].specularReflectance;
    float refractionIndex         = _materials[report.materialId].refractionIndex;
    float absorbtionIndex         = _materials[report.materialId].absorptionIndex;
    float phongExponent           = _materials[report.materialId].phongExponent;

    BRDF brdf = _materials[report.materialId].brdf;
    bool hasBrdf   = _materials[report.materialId].hasBrdf;
    bool gammaflag = _materials[report.materialId].degammaFlag;

    for(size_t i=0; i<_lightPointerVector.size(); i++)
    {

        if(gammaflag)
        {
            result += _lightPointerVector[i]->ComputeDiffuseSpecular(ray, diffuseReflectance, specularReflectance, phongExponent,
                                                                 report, 0.00001, 2000, _intersectionTestEpsilon, _shadowRayEpsilon, 
                                                                 true, ray.time, _objectPointerVector, gammaflag, _activeCamera.gamma, hasBrdf, brdf, refractionIndex, absorbtionIndex);
        }
        else
            result += _lightPointerVector[i]->ComputeDiffuseSpecular(ray, diffuseReflectance, specularReflectance, phongExponent,
                                                                 report, 0.00001, 2000, _intersectionTestEpsilon, _shadowRayEpsilon, 
                                                                 true, ray.time, _objectPointerVector, gammaflag, 0, hasBrdf, brdf, refractionIndex, absorbtionIndex);            
        

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
    bool flag = false;

    if(TestWorldIntersection(ray, r, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
    {
        glm::vec3 pixel(0.0);
        if(r.diffuseActive && r.replaceAll)
            pixel = r.texDiffuseReflectance;
        else if(r.isLight)
            pixel = r.radiance;
        else
            pixel += ComputeAmbientComponent(r) + ComputeDiffuseSpecular(r, ray) + RecursiveTrace(ray, r, 0, false);
        
        if(std::isnan(pixel.x))
        {
            pixel = glm::vec3(0.0,0.0,0.0);
        }
        result.resultColor = pixel;
        result.hit = true;
        return result;
    }

    result.resultColor = glm::clamp(_backgroundColor, glm::vec3(0.0f), glm::vec3(FLT_MAX));
    result.hit = false;
    return result;

}

RayTraceResult Scene::PathTrace(const Ray& ray, bool backfaceCulling, int recursionDepth)
{

    RayTraceResult result;
    result.hit = false;
    result.resultColor = glm::vec3(0.0f);
    float rrProb = 1.0f;
    float rayEnergyLoseFactor = 0.95;
    // Checking stopping conditions
    if(_activeCamera.russianRoulette && recursionDepth > this->_maxRecursionDepth)
    {
        float randomNumber = randomVariableGenerator->Generate();
        float q = 1 - ray.rayThroughput;
        if(randomNumber <= q)
        {
            result.hit = false;
            result.resultColor = glm::vec3(0.0f);
            return result;
        }

        rrProb = 1 - q;
            
    }
    else if(!_activeCamera.russianRoulette && recursionDepth > this->_maxRecursionDepth)
    {
        result.hit = false;
        result.resultColor = glm::vec3(0.0f);
        return result;
    }

    IntersectionReport r;
    bool flag = false;

    if(TestWorldIntersection(ray, r, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
    {
        if(r.diffuseActive && r.replaceAll)
        {
            result.hit = true;
            result.resultColor = r.texDiffuseReflectance;
        }
        else if(r.isLight)
        {
            result.hit = true;
            result.resultColor = r.radiance;
            return result;
        }
        else
        {
            glm::vec3 bounceRayOrigin(0.0f);
            glm::vec3 bounceRayDirection(0.0f);

            Ray bounceRay;


            glm::vec3 attenuation(1.0);

            if(ray.materialIdCurrentlyIn != -1)
            {
                float dist = glm::length(ray.origin - r.intersection);

                float cx = _materials[ray.materialIdCurrentlyIn].absorptionCoefficient.x;
                float cy = _materials[ray.materialIdCurrentlyIn].absorptionCoefficient.y;
                float cz = _materials[ray.materialIdCurrentlyIn].absorptionCoefficient.z;

                attenuation.x = std::pow((float)EULER, -cx * dist);
                attenuation.y = std::pow((float)EULER, -cy * dist);
                attenuation.z = std::pow((float)EULER, -cz * dist);

            }

            // Diffuse
            if(_materials[r.materialId].type == -1)
            {
                    
                glm::vec3 reflectedRayOrigin = r.intersection + r.normal*_shadowRayEpsilon;
                glm::vec3 reflectedRayDir;
                float probabilityInv = 0;
                if(_activeCamera.importanceSampling)
                {
                    reflectedRayDir = directionSampler->importanceSample(r.normal);
                    probabilityInv  = M_PI / std::max(0.1f, glm::dot(reflectedRayDir, r.normal));
                }
                else
                {
                    reflectedRayDir = directionSampler->uniformSample(r.normal);
                    probabilityInv  = 2 * M_PI;                         
                }
                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;
                bool directionSuitable = true;

                if(_activeCamera.nextEventEstimation)
                {
                    IntersectionReport report;

                    if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                    {
                        Object* obj = (Object*) report.hitObject;
                        for(auto element : _lightObjectPointerVector)
                        {
                            if(obj == element)
                                directionSuitable = false;
                        }
                    }

                    if(directionSuitable)
                    {
                        result.resultColor = probabilityInv * getReflectance(ray, reflectedRayDir, 
                                                            _materials[r.materialId].diffuseReflectance, 
                                                            _materials[r.materialId].specularReflectance,
                                                            _materials[r.materialId].phongExponent,
                                                            r, _materials[r.materialId].degammaFlag,
                                                            _activeCamera.gamma,
                                                            _materials[r.materialId].hasBrdf, _materials[r.materialId].brdf,
                                                            _materials[r.materialId].refractionIndex, _materials[r.materialId].absorptionIndex) *
                                                            PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor;
                        result.resultColor += ComputeDiffuseSpecular(r, ray);
                        result.resultColor /= rrProb;
                        result.hit = true;
                    }
                    else
                    {
                        result.resultColor = ComputeDiffuseSpecular(r, ray);
                        result.hit = true;
                    }
                
                }
                else
                {
                    result.resultColor = probabilityInv * getReflectance(ray, reflectedRayDir, 
                                                        _materials[r.materialId].diffuseReflectance, 
                                                        _materials[r.materialId].specularReflectance,
                                                        _materials[r.materialId].phongExponent,
                                                        r, _materials[r.materialId].degammaFlag,
                                                        _activeCamera.gamma,
                                                        _materials[r.materialId].hasBrdf, _materials[r.materialId].brdf,
                                                        _materials[r.materialId].refractionIndex, _materials[r.materialId].absorptionIndex) *
                                                        PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor;
                    result.hit = true;
                    result.resultColor /= rrProb;          
                }

            }
            // Dielectric
            else if(_materials[r.materialId].type == 1)
            {

                // Ray is entering
                if(glm::dot(ray.direction, r.normal) < 0)
                {
                    glm::vec3 reflectedRayOrigin = r.intersection + r.normal * 0.01f;
                    glm::vec3 reflectedRayDir    = glm::normalize(glm::reflect(ray.direction, r.normal));

                    float cosTheta = glm::dot(-ray.direction, r.normal);
                    float coeffRatio = 1/_materials[r.materialId].refractionIndex;

                    float cosPhiSquared = (1 - coeffRatio*coeffRatio * (1 - cosTheta*cosTheta));

                    // Reflection and transmission both occur
                    if(cosPhiSquared >= 0)
                    {
                        float cosPhi = std::sqrt(cosPhiSquared);

                        // Building transmitted ray
                        glm::vec3 transmittedRayOrigin = r.intersection - r.normal * 0.01f;            
                        glm::vec3 transmittedRayDir = (ray.direction + r.normal*cosTheta)*coeffRatio - r.normal*cosPhi;

                        Ray tRay(transmittedRayOrigin, transmittedRayDir);
                        tRay.mediumCoeffNow = _materials[r.materialId].refractionIndex;
                        tRay.mediumCoeffBefore = ray.mediumCoeffNow;
                        tRay.isRefracting = true;
                        tRay.materialIdCurrentlyIn = r.materialId;
                        tRay.time = ray.time;
                        tRay.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;

                        Ray reflected(reflectedRayOrigin, reflectedRayDir);
                        reflected.isRefracting = ray.isRefracting;
                        reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                        reflected.mediumCoeffNow = ray.mediumCoeffNow;
                        reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;
                        reflected.time = ray.time;
                        reflected.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;

                        float rRpar = (tRay.mediumCoeffNow*cosTheta - 1*cosPhi)/
                                    (tRay.mediumCoeffNow*cosTheta + 1*cosPhi);

                        float rPpar = (1*cosTheta - tRay.mediumCoeffNow*cosPhi)/
                                    (1*cosTheta + tRay.mediumCoeffNow*cosPhi);

                        float reflectionRatio = (rPpar*rPpar + rRpar*rRpar)/2;
                        float transmissionRatio = 1 - reflectionRatio;

                        bool directionSuitable = true;

                        if(_activeCamera.nextEventEstimation)
                        {
                            IntersectionReport report;

                            if(TestWorldIntersection(reflected, report, 0, 9000, _intersectionTestEpsilon, backfaceCulling))
                            {
                                Object* obj = (Object*) report.hitObject;
                                for(auto element : _lightObjectPointerVector)
                                {
                                    if(obj == element)
                                        directionSuitable = false;
                                }
                            }

                            if(directionSuitable)
                            {
                                result.resultColor = reflectionRatio * PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor;
                                result.resultColor += ComputeDiffuseSpecular(r, ray);
                                result.resultColor /= rrProb;                                
                            }
                            else
                            {
                                result.resultColor = ComputeDiffuseSpecular(r, ray);
                                result.resultColor /= rrProb;                              
                            }
                        
                        }
                        else
                        {
                            result.resultColor = reflectionRatio * PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor;
                        }                        

                                                                 
                        result.resultColor +=  transmissionRatio * PathTrace(tRay, backfaceCulling, recursionDepth + 1).resultColor;
                        result.resultColor /= rrProb;
                        result.hit = true;

                        
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
                        reflected.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;
                        bool directionSuitable = true;

                        if(_activeCamera.nextEventEstimation)
                        {
                            IntersectionReport report;

                            if(TestWorldIntersection(reflected, report, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                            {
                                Object* obj = (Object*) report.hitObject;
                                for(auto element : _lightObjectPointerVector)
                                {
                                    if(obj == element)
                                        directionSuitable = false;
                                }
                            }

                            if(directionSuitable)
                            {
                                result.resultColor =  PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor;
                                result.resultColor += ComputeDiffuseSpecular(r, ray);
                                result.resultColor /= rrProb;                                
                            }
                            else
                            {
                                result.resultColor = ComputeDiffuseSpecular(r, ray);
                                result.resultColor /= rrProb;                               
                            }
                        
                        }
                        else
                        {
                            result.resultColor +=  PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor;
                            result.resultColor /= rrProb;                                                                
                        }  

                        result.hit = true;                            
                    }                         
                }

                // Ray is exiting
                else if(glm::dot(ray.direction, r.normal) > 0)
                {
                    glm::vec3 invertedNormal = -r.normal;

                    glm::vec3 reflectedRayOrigin = r.intersection + invertedNormal * 0.01f;
                    glm::vec3 reflectedRayDir    = glm::reflect(ray.direction, invertedNormal);

                    float cosTheta = glm::dot(-ray.direction, invertedNormal);
                    float coeffRatio = ray.mediumCoeffNow/1;

                    float cosPhiSquared = (1 - coeffRatio*coeffRatio * (1 - cosTheta*cosTheta));

                    // Reflection and transmission both occur
                    if(cosPhiSquared >= 0)
                    {
                        float cosPhi = std::sqrt(cosPhiSquared);

                        // Building transmitted ray
                        glm::vec3 transmittedRayOrigin = r.intersection - invertedNormal * 0.01f;            
                        glm::vec3 transmittedRayDir = (ray.direction + invertedNormal*cosTheta)*coeffRatio - invertedNormal*cosPhi;

                        Ray tRay(transmittedRayOrigin, transmittedRayDir);
                        tRay.mediumCoeffBefore = ray.mediumCoeffNow;
                        tRay.mediumCoeffNow = 1;
                        tRay.isRefracting = false;
                        tRay.materialIdCurrentlyIn = -1;
                        tRay.time = ray.time;
                        tRay.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;

                        Ray reflected(reflectedRayOrigin, reflectedRayDir);
                        reflected.isRefracting = ray.isRefracting;
                        reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                        reflected.mediumCoeffNow = ray.mediumCoeffNow;
                        reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;
                        reflected.time = ray.time;
                        reflected.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;

                        float rRpar = (1*cosTheta - tRay.mediumCoeffBefore*cosPhi)/
                                    (1*cosTheta + tRay.mediumCoeffBefore*cosPhi);

                        float rPpar = (tRay.mediumCoeffBefore*cosTheta - 1*cosPhi)/
                                    (tRay.mediumCoeffBefore*cosTheta + 1*cosPhi);

                        float reflectionRatio = (rPpar*rPpar + rRpar*rRpar)/2;
                        float transmissionRatio = 1 - reflectionRatio;

                        result.resultColor = reflectionRatio * PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor * attenuation
                                                                                                                                 
                                                                                            +
                                                                 
                                             transmissionRatio * PathTrace(tRay, backfaceCulling, recursionDepth + 1).resultColor * attenuation;
                        result.resultColor /= rrProb;                                             
                        result.hit = true;

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
                        reflected.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;
                            
                        result.resultColor = (PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor) * attenuation;
                        result.resultColor /= rrProb;                                                              

                        result.hit = true;                       
                           
                    } 
                }


            }

            // Conductor
            else if(_materials[r.materialId].type == 2)
            {
                glm::vec3 reflectedRayOrigin = r.intersection + r.normal*_shadowRayEpsilon;
                glm::vec3 reflectedRayDir    = glm::normalize(glm::reflect(ray.direction, r.normal));
                float cosTheta = glm::dot(ray.direction, r.normal);
                float aI = _materials[r.materialId].absorptionIndex;
                float rI = _materials[r.materialId].refractionIndex;


                float rS = ((rI*rI + aI*aI) - 2*rI*cosTheta + (cosTheta*cosTheta))/
                        ((rI*rI + aI*aI) + 2*rI*cosTheta + (cosTheta*cosTheta));

                float rP = ((rI*rI + aI*aI)*(cosTheta*cosTheta) - 2*rI*cosTheta + 1)/
                        ((rI*rI + aI*aI)*(cosTheta*cosTheta) + 2*rI*cosTheta + 1);

                float reflectionRatio = (rS + rP)/2;

                OrthonormalBasis basis = GiveOrthonormalBasis(reflectedRayDir);
                float randomOffsetU = glossyReflectionVarGenerator->Generate();
                float randomOffsetV = glossyReflectionVarGenerator->Generate();

                reflectedRayDir = reflectedRayDir + _materials[r.materialId].roughness*(randomOffsetU*basis.u + randomOffsetV*basis.v);
                
                Ray reflected(reflectedRayOrigin, reflectedRayDir);
                reflected.isRefracting = ray.isRefracting;
                reflected.mediumCoeffBefore = ray.mediumCoeffBefore;
                reflected.mediumCoeffNow    = ray.mediumCoeffNow;
                reflected.materialIdCurrentlyIn = ray.materialIdCurrentlyIn;
                reflected.time = ray.time;

                reflected.rayThroughput = ray.rayThroughput * rayEnergyLoseFactor;
                bool directionSuitable = true;

                if(_activeCamera.nextEventEstimation)
                {
                    IntersectionReport report;

                    if(TestWorldIntersection(reflected, report, 0, 6000, _intersectionTestEpsilon, backfaceCulling))
                    {
                        Object* obj = (Object*) report.hitObject;
                        for(auto element : _lightObjectPointerVector)
                        {
                            if(obj == element)
                                directionSuitable = false;
                        }
                    }

                    if(directionSuitable)
                    {
                        result.resultColor = reflectionRatio * _materials[r.materialId].mirrorReflectance * PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor * attenuation;
                        result.resultColor += ComputeDiffuseSpecular(r, ray);
                        result.resultColor /= rrProb;
                        //std::cout << _materials[r.materialId].mirrorReflectance.x << " " << _materials[r.materialId].mirrorReflectance.y << " " << _materials[r.materialId].mirrorReflectance.z << " "<< reflectionRatio <<  std::endl;
                        result.hit = true;
                    }
                    else
                    {
                        result.resultColor = ComputeDiffuseSpecular(r, ray);
                        result.resultColor /= rrProb;                        
                        result.hit = true;                        
                    }
                        
                }
                else
                {
                    result.resultColor += reflectionRatio * _materials[r.materialId].mirrorReflectance * PathTrace(reflected, backfaceCulling, recursionDepth + 1).resultColor * attenuation;
                    result.resultColor /= rrProb;                   
                    result.hit = true;                    
                }          
                    
            }            


        }

    }

    if(std::isnan(result.resultColor.x) || std::isnan(result.resultColor.y) || std::isnan(result.resultColor.z))
    {
        result.resultColor = glm::vec3(0.0,0.0,0.0);
    }    

    return result;
    
}

glm::vec3 Scene::TraceAndFilter(std::vector<RayWithWeigth> rwwVector, int x, int y)
{
    float stdDev = 1.f/6.f;
    glm::vec3 result(0.0);

    glm::vec3 weightedSum(0.f);
    glm::vec3 totalWeight(0.f);

    for(size_t i=0; i<rwwVector.size(); i++)
    {
        RayTraceResult rtResult;

        if(_activeCamera.lightingMode == LightingMode::DIRECT_LIGHTING)
            rtResult = RayTrace(rwwVector[i].r, false);
        else if(_activeCamera.lightingMode == LightingMode::PATH_TRACING)
            rtResult = PathTrace(rwwVector[i].r, false, 0);

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
        attenuation.x = std::pow((float)EULER, std::log(_materials[ray.materialIdCurrentlyIn].absorptionCoefficient.x) *dist);
        attenuation.y = std::pow((float)EULER, std::log(_materials[ray.materialIdCurrentlyIn].absorptionCoefficient.y) *dist);
        attenuation.z = std::pow((float)EULER, std::log(_materials[ray.materialIdCurrentlyIn].absorptionCoefficient.z) *dist);
                       
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
        else if(_environmentLights.size() > 0)
        {
            glm::vec3 l = reflected.direction;
            glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
            glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
            glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

            float theta = std::acos(glm::dot(l,v));
            float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

            float tU = (-phi + M_PI) / (2 * M_PI);
            float tV = theta / M_PI;

            glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
            result += attenuation * _materials[iR.materialId].mirrorReflectance * envColor;
              
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
                else if(_environmentLights.size() > 0)
                {
                    glm::vec3 l = reflected.direction;
                    glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
                    glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
                    glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

                    float theta = std::acos(glm::dot(l,v));
                    float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

                    float tU = (-phi + M_PI) / (2 * M_PI);
                    float tV = theta / M_PI;

                    glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
                    result += reflectionRatio * envColor;
                    
                }

                IntersectionReport report2;
                if(TestWorldIntersection(tRay, report2, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += attenuation * transmissionRatio * (RecursiveTrace(tRay, report2, bounce + 1, backfaceCulling));
                }
                else if(_environmentLights.size() > 0)
                {
                    glm::vec3 l = tRay.direction;
                    glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
                    glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
                    glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

                    float theta = std::acos(glm::dot(l,v));
                    float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

                    float tU = (-phi + M_PI) / (2 * M_PI);
                    float tV = theta / M_PI;

                    glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
                    result += transmissionRatio * envColor;
                    
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
                else if(_environmentLights.size() > 0)
                {
                    glm::vec3 l = reflected.direction;
                    glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
                    glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
                    glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

                    float theta = std::acos(glm::dot(l,v));
                    float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

                    float tU = (-phi + M_PI) / (2 * M_PI);
                    float tV = theta / M_PI;

                    glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
                    result += envColor;
                    
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
                else if(_environmentLights.size() > 0)
                {
                    glm::vec3 l = reflected.direction;
                    glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
                    glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
                    glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

                    float theta = std::acos(glm::dot(l,v));
                    float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

                    float tU = (-phi + M_PI) / (2 * M_PI);
                    float tV = theta / M_PI;

                    glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
                    result += reflectionRatio * attenuation * envColor;
                    
                }

                IntersectionReport report2;
                if(TestWorldIntersection(tRay, report2, 0, 2000, _intersectionTestEpsilon, backfaceCulling))
                {
                    result += transmissionRatio * attenuation * (ComputeAmbientComponent(report2) + ComputeDiffuseSpecular(report2, tRay) + RecursiveTrace(tRay, report2, bounce + 1, backfaceCulling));
                }
                else if(_environmentLights.size() > 0)
                {
                    glm::vec3 l = tRay.direction;
                    glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
                    glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
                    glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

                    float theta = std::acos(glm::dot(l,v));
                    float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

                    float tU = (-phi + M_PI) / (2 * M_PI);
                    float tV = theta / M_PI;

                    glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
                    result += transmissionRatio * attenuation * envColor;
                    
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
                else if(_environmentLights.size() > 0)
                {
                    glm::vec3 l = reflected.direction;
                    glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
                    glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
                    glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

                    float theta = std::acos(glm::dot(l,v));
                    float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

                    float tU = (-phi + M_PI) / (2 * M_PI);
                    float tV = theta / M_PI;

                    glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
                    result += attenuation * envColor;
                    
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
        else if(_environmentLights.size() > 0)
        {
            glm::vec3 l = reflected.direction;
            glm::vec3 v = glm::vec3(0.0, 1.0, 0.0);
            glm::vec3 u = glm::vec3(1.0, 0.0, 0.0);
            glm::vec3 w = glm::vec3(0.0, 0.0, 1.0);

            float theta = std::acos(glm::dot(l,v));
            float phi   = std::atan2(glm::dot(l,w), glm::dot(l,u));

            float tU = (-phi + M_PI) / (2 * M_PI);
            float tV = theta / M_PI;

            glm::vec3 envColor = _environmentLights[0].hdrTexture.Fetch(tU, tV);
            result += attenuation *  _materials[iR.materialId].mirrorReflectance * envColor;
                    
        }          
              
    }

    return result;
}