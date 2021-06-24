#ifndef __UTILS_H__
#define __UTILS_H__

#include <Structures.h>
#include <Mesh.h>
#include <Triangle.h>
#include <MeshInstance.h>
#include <Sphere.h>
#include <Texture.h>
#include <Image.h>
#include <sstream>
#include <happly.h>
#include <map>

#include <Light.h>
#include <AreaLight.h>
#include <DirectionalLight.h>
#include <PointLight.h>


const double EULER =  2.71828182845904523536;

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

inline std::vector<std::string> split(std::string text)
{

    std::vector<std::string> vec;
    std::stringstream ss(text);
    for(std::string s; ss >> s;)
    {
        vec.push_back(s);
    }

    return vec;
}

template<typename T>
inline T clamp(const T& val, const T& low, const T& high)
{
    T result = val;

    if(result < low)
        result = low;
    else if(result > high)
        result = high;

    return result;
}


inline void SceneReadConstants(tinyxml2::XMLNode* root, glm::vec3& _backgroundColor, float& _shadowRayEpsilon, float& _intersectionTestEpsilon, int& _maxRecursionDepth)
{
    std::stringstream stream;
    auto element = root->FirstChildElement("BackgroundColor");
    if(element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0 0 0" << std::endl;
    }

    stream >> _backgroundColor.r >> _backgroundColor.g >> _backgroundColor.b;

    element = root->FirstChildElement("ShadowRayEpsilon");
    if(element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0.01" << std::endl;
    }
    stream >> _shadowRayEpsilon;

    element = root->FirstChildElement("IntersectionTestEpsilon");
    if(element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0.0001" << std::endl;
    }
    stream >> _intersectionTestEpsilon;

    element = root->FirstChildElement("MaxRecursionDepth");
    if(element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0" << std::endl;
    }
    stream >> _maxRecursionDepth;

    stream.clear();    
}

inline void SceneReadCameras(tinyxml2::XMLNode* root, std::vector<Camera>& _cameras, std::vector<std::string>& imageNames, std::string& _imageName)
{
    std::stringstream stream;    
    auto element = root->FirstChildElement("Cameras");
    element = element->FirstChildElement("Camera");
    Camera camera;
    while(element)
    {

        if(element->Attribute("type") && strcmp(element->Attribute("type"), "lookAt") == 0)
        {
            float fovY;
            glm::vec3 gazePoint;

            auto child = element->FirstChildElement("Position");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("GazePoint");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("Up");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("FovY");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("NearDistance");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("ImageResolution");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("ImageName");
            stream << child->GetText() << std::endl;

            stream >> camera.position.x >> camera.position.y >> camera.position.z;
            stream >> gazePoint.x >> gazePoint.y >> gazePoint.z;
            stream >> camera.up.x >> camera.up.y >> camera.up.z;
            stream >> fovY;
            stream >> camera.nearDistance;
            stream >> camera.imageResolution.x >> camera.imageResolution.y;

            float l, r, b, t;

            t = std::tan((fovY*M_PI)/(2*180))*camera.nearDistance;
            r = (camera.imageResolution.x / camera.imageResolution.y) * t;
            l = -r;
            b = -t;

            glm::vec3 gaze = glm::normalize(gazePoint - camera.position);
            camera.gaze = gaze;

            glm::vec3 w = -glm::normalize(camera.gaze);
            camera.v = glm::normalize(glm::cross(camera.up, w));    
            camera.up   = glm::normalize(glm::cross(w, camera.v));
            camera.nearPlane = glm::vec4(l,r,b,t);            
        }
        else
        {
            auto child = element->FirstChildElement("Position");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("Gaze");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("Up");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("NearPlane");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("NearDistance");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("ImageResolution");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("ImageName");
            stream << child->GetText() << std::endl;

            stream >> camera.position.x >> camera.position.y >> camera.position.z;
            stream >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
            stream >> camera.up.x >> camera.up.y >> camera.up.z;
            stream >> camera.nearPlane.x >> camera.nearPlane.y >> camera.nearPlane.z >> camera.nearPlane.w;
            stream >> camera.nearDistance;
            stream >> camera.imageResolution.x >> camera.imageResolution.y;

            // normalize gaze and up and compute v

            glm::vec3 w = -glm::normalize(camera.gaze);
            camera.v = glm::normalize(glm::cross(camera.up, w));    
            camera.up   = glm::normalize(glm::cross(w, camera.v));
            camera.gaze = glm::normalize(camera.gaze);

        }
        
        std::string imageName;
        stream >> imageName;
        imageNames.push_back(imageName);
        _imageName = imageName;

        int dotIndex = imageName.find(".");
        std::string extensionType = imageName.substr(dotIndex, imageName.size());

        if(extensionType == "exr")
            camera.renderMode = RenderMode::HDR;
        else
            camera.renderMode = RenderMode::CLASSIC;

        auto child = element->FirstChildElement("Tonemap");
        if(child)
        {
            auto element = child->FirstChildElement("TMO");
            if(element)
            {
                if(std::strcmp(element->GetText(), "Photographic") == 0)
                    camera.tmo = TMO::PHOTOHRAPHIC;
            }
            else
                camera.tmo = TMO::PHOTOHRAPHIC;

            element = child->FirstChildElement("TMOOptions");
            if(element)
            {
                stream << element->GetText() << std::endl;
                stream >> camera.keyValue >> camera.burn_percentage;
            }
            else
            {
                camera.keyValue = 0.18;
                camera.burn_percentage = 1;
            }

            element = child->FirstChildElement("Saturation");
            if(element)
            {
                stream << element->GetText() << std::endl;
                stream >> camera.saturation;
            }
            else
            {
                camera.saturation = 1;
            }

            element = child->FirstChildElement("Gamma");

            if(element)
            {
                stream << element->GetText() << std::endl;
                stream >> camera.gamma;
            }
            else
            {
                camera.gamma = 2.2;
            }
        }

        child = element->FirstChildElement("NumSamples");
        int sampleNumber = 1;
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> sampleNumber;
        }
        camera.sampleNumber = sampleNumber;


        child = element->FirstChildElement("FocusDistance");
        float focusDistance = 0;
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> focusDistance;
        }

        child = element->FirstChildElement("ApertureSize");
        float apertureSize = 0;
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> apertureSize;
        }

        camera.focusDistance = focusDistance;
        camera.apertureSize = apertureSize;
        _cameras.push_back(camera);
        element = element->NextSiblingElement("Camera");
    }
    stream.clear();    
}


inline void SceneReadLights(tinyxml2::XMLNode* root, std::vector<PointLight>& _pointLights, std::vector<AreaLight>& _areaLights, std::vector<SphericalDirectionalLight>& _environmentLights, std::vector<DirectionalLight>& _directionalLights, std::vector<SpotLight>& _spotLights, std::vector<Image*>& _images, glm::vec3& _ambientLight)
{
    std::stringstream stream;
    // Get Lights
    auto element = root->FirstChildElement("Lights");
    auto child = element->FirstChildElement("AmbientLight");
    stream << child->GetText() << std::endl;
    stream >> _ambientLight.x >> _ambientLight.y >> _ambientLight.z;

    element = element->FirstChildElement("PointLight");
    while(element)
    {
        PointLight pointLight;        
        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Intensity");
        stream << child->GetText() << std::endl;

        stream >> pointLight.position.x >> pointLight.position.y >> pointLight.position.z;
        stream >> pointLight.intensity.r >> pointLight.intensity.g >> pointLight.intensity.b;

        _pointLights.push_back(pointLight);
        element = element->NextSiblingElement("PointLight");
    }
    stream.clear();

    element = root->FirstChildElement("Lights");
    element = element->FirstChildElement("AreaLight");
    while(element)
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 radiance;
        float extent;

        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Normal");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Radiance");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Size");
        stream << child->GetText() << std::endl;

        stream >> position.x >> position.y >> position.z;
        stream >> normal.x >> normal.y >> normal.z;
        stream >> radiance.x >> radiance.y >> radiance.z;
        stream >> extent;

        AreaLight areaLight(position, radiance, normal, extent);

        _areaLights.push_back(areaLight);
        element = element->NextSiblingElement("AreaLight");
    }

    element = root->FirstChildElement("Lights");
    element = element->FirstChildElement("SphericalDirectionalLight");
    while(element)
    {
        int imageId = 1;
        SphericalDirectionalLight environmentLight;
        child = element->FirstChildElement("ImageId");
        stream << child->GetText() << std::endl;
        stream >> imageId;
        environmentLight.hdrTexture.BindImage(_images[imageId - 1]);

        _environmentLights.push_back(environmentLight);
        element = element->NextSiblingElement("SphericalDirectionalLight");
    }

    element = root->FirstChildElement("Lights");
    element = element->FirstChildElement("DirectionalLight");
    while(element)
    {
        DirectionalLight directionalLight;
        std::stringstream ss;
        
        child = element->FirstChildElement("Direction");
        ss << child->GetText() << std::endl;
        ss >> directionalLight.direction.x >> directionalLight.direction.y >> directionalLight.direction.z;

        directionalLight.direction = glm::normalize(directionalLight.direction);

        child = element->FirstChildElement("Radiance");
        ss << child->GetText() << std::endl;
        ss >> directionalLight.radiance.x >> directionalLight.radiance.y >> directionalLight.radiance.z;


        _directionalLights.push_back(directionalLight);
        element = element->NextSiblingElement("DirectionalLight");
    }

    element = root->FirstChildElement("Lights");
    element = element->FirstChildElement("SpotLight");
    while(element)
    {
        SpotLight spotLight;
        std::stringstream ss;

        child = element->FirstChildElement("Position");
        ss << child->GetText() << std::endl;
        ss >> spotLight.position.x >> spotLight.position.y >> spotLight.position.z;

        child = element->FirstChildElement("Direction");
        ss << child->GetText() << std::endl;
        ss >> spotLight.direction.x >> spotLight.direction.y >> spotLight.direction.z;
        spotLight.direction = glm::normalize(spotLight.direction);

        child = element->FirstChildElement("Intensity");
        ss << child->GetText() << std::endl;
        ss >> spotLight.intensity.x >> spotLight.intensity.y >> spotLight.intensity.z;

        child = element->FirstChildElement("CoverageAngle");
        ss << child->GetText() << std::endl;
        ss >> spotLight.coverageAngle;

        child = element->FirstChildElement("FalloffAngle");
        ss << child->GetText() << std::endl;
        ss >> spotLight.falloffAngle;

        _spotLights.push_back(spotLight);
        element = element->NextSiblingElement("SpotLight");
    }

    stream.clear();
}

inline void SceneReadMaterials(tinyxml2::XMLNode* root, std::vector<Material>& _materials)
{
    std::stringstream stream;
    auto element = root->FirstChildElement("Materials");
    element = element->FirstChildElement("Material");
    Material material;
    while(element)
    {
     
        auto child = element->FirstChildElement("AmbientReflectance");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("DiffuseReflectance");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("SpecularReflectance");
        stream << child->GetText() << std::endl;

        child = element->FirstChildElement("MirrorReflectance");
        if(child)
        {
            stream << child->GetText() << std::endl;
        }
        else
        {
            stream << "0 0 0" << std::endl;
        }

        child = element->FirstChildElement("AbsorptionCoefficient");
        if(child)
        {
            stream << child->GetText() << std::endl;
        }
        else
        {
            stream << "0 0 0" << std::endl;
        }

        child = element->FirstChildElement("RefractionIndex");
        if(child)
        {
            stream << child->GetText() << std::endl;
        }
        else
        {
            stream << "0" << std::endl;
        }

        child = element->FirstChildElement("AbsorptionIndex");
        if(child)
        {
            stream << child->GetText() << std::endl;
        }
        else
        {
            stream << "0" << std::endl;
        }
        
        child = element->FirstChildElement("PhongExponent");
        if(child)
        {
            stream << child->GetText() << std::endl;
        }
        else
        {
            stream << "1" << std::endl;
        }

        child = element->FirstChildElement("Roughness");
        if(child)
        {
            stream << child->GetText() << std::endl;
        }
        else
        {
            stream << "0" << std::endl;
        }

        stream >> material.ambientReflectance.x >> material.ambientReflectance.y >> material.ambientReflectance.z;
        stream >> material.diffuseReflectance.x >> material.diffuseReflectance.y >> material.diffuseReflectance.z;
        stream >> material.specularReflectance.x >> material.specularReflectance.y >> material.specularReflectance.z;
        stream >> material.mirrorReflectance.x >> material.mirrorReflectance.y >> material.mirrorReflectance.z;
        stream >> material.absorptionCoefficient.x >> material.absorptionCoefficient.y >> material.absorptionCoefficient.z;
        stream >> material.refractionIndex;
        stream >> material.absorptionIndex;
        stream >> material.phongExponent;
        stream >> material.roughness;

        material.type = -1;

        if(element->Attribute("type"))
        {
            const char* type = element->Attribute("type");
            if(strcmp(type, "conductor") == 0)
            {
                material.type = 2;
            }
            else if(strcmp(type, "dielectric") == 0)
            {
                material.type = 1;
            }
            else if(strcmp(type, "mirror") == 0)
            {
                material.type = 0;
            }
        }

        _materials.push_back(material);
        element = element->NextSiblingElement("Material");
    }
    stream.clear();
}

inline void SceneReadVertexData(tinyxml2::XMLNode* root, std::vector<glm::vec3>& _vertexData)
{
    std::stringstream stream;
    auto element = root->FirstChildElement("VertexData");
    if(element)
        stream << element->GetText() << std::endl;
    glm::vec3 vertex;
    while(!(stream >> vertex.x).eof())
    {
        stream >> vertex.y >> vertex.z;
        _vertexData.push_back(vertex);
    }
    stream.clear();
}

inline void SceneReadImages(tinyxml2::XMLNode* imagesNode, std::vector<Image*>& _images)
{
    std::stringstream stream;
    auto element = imagesNode->FirstChildElement("Image");

    while(element)
    {
        std::string filename;
        
        stream << element->GetText() << std::endl;
        stream >> filename;
        
        std::string path = std::string(ROOT_DIR) + "assets/scenes/"+ filename;
        Image* newImage = new Image(path.c_str());
        _images.push_back(newImage);

        element = element->NextSiblingElement("Image");
    }
}

inline void SceneReadTextureMaps(tinyxml2::XMLNode* texturesNode, std::vector<Image*>& _images, std::vector<Texture*>& _textures, int& _backgroundTextureIndex)
{
    auto element = texturesNode->FirstChildElement("TextureMap");

    // TODO: BAKKKKK!!
    while(element)
    {
        if(element->Attribute("type"))
        {
            std::string texType(element->Attribute("type"));

            Texture* tex = new Texture();
            tex->mapType = MapType::DIFFUSE_MAP;

            if(texType == "perlin")
                tex->type = TextureType::PERLIN;
            else if(texType == "image")
                tex->type = TextureType::IMAGE;
            else if(texType == "checkerboard")
                tex->type = TextureType::CHECKERBOARD;
            else
                tex->type = TextureType::PERLIN;


            auto child = element->FirstChildElement("DecalMode");
            if(child)
            {
                if(std::strcmp(child->GetText(), "replace_kd") == 0)
                    tex->decalMode = DecalMode::REPLACE_KD;
                else if(std::strcmp(child->GetText(), "replace_background") == 0)
                    tex->decalMode = DecalMode::REPLACE_BACKGROUND;
                else if(std::strcmp(child->GetText(), "blend_kd") == 0)
                    tex->decalMode = DecalMode::BLEND_KD;
                else if(std::strcmp(child->GetText(), "replace_all") == 0)
                    tex->decalMode = DecalMode::REPLACE_ALL;
                else if(std::strcmp(child->GetText(), "replace_normal") == 0)
                {
                    tex->decalMode = DecalMode::REPLACE_NORMAL;
                    tex->mapType   = MapType::NORMAL_MAP;
                }
                else if(std::strcmp(child->GetText(), "bump_normal") == 0)
                {
                    tex->decalMode = DecalMode::BUMP_NORMAL;
                    tex->mapType   = MapType::BUMP_MAP;
                }
                
            }
            else
            {
                tex->decalMode = DecalMode::REPLACE_ALL;
            }

            child = element->FirstChildElement("NoiseConversion");
            if(child)
            {
                if(std::strcmp(child->GetText(), "linear") == 0)
                {
                    tex->noiseConversion = NoiseConversionType::LINEAR;
                }
                else if(std::strcmp(child->GetText(), "absval") == 0)
                {
                    tex->noiseConversion = NoiseConversionType::ABSVAL;
                }
            }
            else
            {
                tex->noiseConversion = NoiseConversionType::LINEAR;
            }

            child = element->FirstChildElement("NoiseScale");
            if(child)
            {
                std::stringstream sstream;
                sstream << child->GetText() << std::endl;
                sstream >> tex->noiseScale;
            }
            else
            {
                tex->noiseScale = 1;
            }

            child = element->FirstChildElement("ImageId");
            if(child)
            {
                std::stringstream ss;
                ss << child->GetText() << std::endl;
                int index = 1;
                ss >> index;
                tex->image = _images[index - 1];
            }

            child = element->FirstChildElement("Interpolation");
            if(child)
            {
                if(std::strcmp(child->GetText(), "nearest") == 0)
                    tex->interpolationType = InterpolationType::NEAREST_NEIGHBOR;
                else
                    tex->interpolationType = InterpolationType::BILINEAR;
            }
            else
                tex->interpolationType = InterpolationType::BILINEAR;

            
            child = element->FirstChildElement("Normalizer");
            if(child)
            {
                std::stringstream ss;
                ss << child->GetText() << std::endl;
                int normalizer = 1;
                ss >> normalizer;
                tex->normalizer = normalizer;
            }
            else
                tex->normalizer = 1;

            child = element->FirstChildElement("BumpFactor");
            if(child)
            {
                std::stringstream ss;
                ss << child->GetText() << std::endl;
                float factor = 0.1;
                ss >> factor;
                tex->bumpFactor = factor;
            }
            else
                tex->bumpFactor = 1;


            child = element->FirstChildElement("BlackColor");
            if(child)
            {
                std::stringstream ss;
                ss << child->GetText() << std::endl;                
                ss >> tex->blackColor.x >> tex->blackColor.y >> tex->blackColor.z;
            }
            else 
                tex->blackColor = glm::vec3(0.4f, 0.4f, 0.4f);

            child = element->FirstChildElement("WhiteColor");
            if(child)
            {
                std::stringstream ss;
                ss << child->GetText() << std::endl;
                ss >> tex->whiteColor.x >> tex->whiteColor.y >> tex->whiteColor.z;
            }
            else
                tex->whiteColor = glm::vec3(10.f, 10.f, 10.f);

            child = element->FirstChildElement("Scale");
            if(child)
            {
                std::stringstream ss;
                ss << child->GetText() << std::endl;
                ss >> tex->scale;
            }
            else 
                tex->scale = 5;

            child = element->FirstChildElement("Offset");
            if(child)
            {
                std::stringstream ss;
                ss << child->GetText() << std::endl;
                ss >> tex->offset;
            }
            else 
                tex->offset = 0;
            
            _textures.push_back(tex);

            if(tex->decalMode == DecalMode::REPLACE_BACKGROUND)
                _backgroundTextureIndex = _textures.size() - 1;
        }

        element = element->NextSiblingElement("TextureMap");
    }
}

inline void SceneReadTextures(tinyxml2::XMLNode* root, std::vector<Image*>& _images, std::vector<Texture*>& _textures, int& _backgroundTextureIndex)
{

    auto textures = root->FirstChildElement("Textures");

    if(textures)
    {
        auto images = textures->FirstChildElement("Images");
        if(images)
        {
            SceneReadImages(images, _images);
        }
        SceneReadTextureMaps(textures, _images, _textures, _backgroundTextureIndex);        
    }

}

inline void SceneReadTexCoordData(tinyxml2::XMLNode* root, std::vector<glm::vec2>& _texCoordData)
{
    std::stringstream stream;
    auto element = root->FirstChildElement("TexCoordData");
    if(element)
    {
        stream << element->GetText() << std::endl;
        glm::vec2 coord;
        while(!(stream >> coord.x).eof())
        {
            stream >> coord.y;
            _texCoordData.push_back(coord);
        }
        stream.clear();
    };
}

inline void SceneReadMeshes(tinyxml2::XMLNode* root, std::vector<Mesh>& _meshes, std::vector<Texture*>& _textures, std::vector<glm::vec3>& _vertexData, std::vector<glm::vec2>& _texCoordData, std::vector<glm::mat4>& _rotationMatrices, std::vector<glm::mat4>& _scalingMatrices, std::vector<glm::mat4>& _translationMatrices, std::vector<glm::mat4>& _compositeMatrices)
{
    std::stringstream stream;
    // Get Meshes
    auto element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Mesh");
    

    while(element)
    {
        bool softShading = false;

        if(element->Attribute("shadingMode"))
        {
            if(std::strcmp(element->Attribute("shadingMode"), "smooth") == 0)
                softShading = true;
        }

        size_t materialId;
        auto child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> materialId;


        child = element->FirstChildElement("Faces");
        stream << child->GetText() << std::endl;

        std::vector<Triangle> triangleList;
        std::vector<glm::vec3> normals;
        std::vector<int> neighborCount;

        if(child->Attribute("plyFile"))
        {
            bool normalsExist = false;
            bool textureCoordsExist = false;

            const char* localPath = child->Attribute("plyFile");
            std::string path = std::string(ROOT_DIR) + "assets/scenes/" + std::string(localPath);

            happly::PLYData plyIn(path);

            std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
            std::vector<std::string> xasd = plyIn.getElement("vertex").getPropertyNames();

            std::vector<float> nx,ny,nz,u,v;

            if(plyIn.getElement("vertex").hasProperty("nx") && plyIn.getElement("vertex").hasProperty("ny") && plyIn.getElement("vertex").hasProperty("nz"))
            {
                nx = plyIn.getElement("vertex").getProperty<float>(std::string("nx"));
                ny = plyIn.getElement("vertex").getProperty<float>(std::string("ny"));
                nz = plyIn.getElement("vertex").getProperty<float>(std::string("nz"));
                normalsExist = true;
            }
            else
            {
                normalsExist = false;
            }

            if(plyIn.getElement("vertex").hasProperty("u") && plyIn.getElement("vertex").hasProperty("v"))
            {
                u = plyIn.getElement("vertex").getProperty<float>(std::string("u"));
                v = plyIn.getElement("vertex").getProperty<float>(std::string("v"));
                textureCoordsExist = true;
            }
            else
            {
                textureCoordsExist = false;
            }
            
            
            std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();


            if(!normalsExist)
            {
                for(size_t i=0; i<vPos.size(); i++)
                {
                    normals.push_back(glm::vec3(0.0));
                    neighborCount.push_back(0);
                }

                for(size_t i=0; i<fInd.size(); i++)
                {
                    glm::vec3 a,b,c;

                    a.x = vPos[fInd[i][0]][0];
                    a.y = vPos[fInd[i][0]][1];
                    a.z = vPos[fInd[i][0]][2];

                    b.x = vPos[fInd[i][1]][0];
                    b.y = vPos[fInd[i][1]][1];
                    b.z = vPos[fInd[i][1]][2];                                

                    c.x = vPos[fInd[i][2]][0];
                    c.y = vPos[fInd[i][2]][1];
                    c.z = vPos[fInd[i][2]][2];

                    glm::vec3 normal = glm::normalize(glm::cross((b-a), (c-a)));

                    normals[fInd[i][0]] += normal;
                    neighborCount[fInd[i][0]] += 1;

                    normals[fInd[i][1]] += normal;
                    neighborCount[fInd[i][1]] += 1;

                    normals[fInd[i][2]] += normal;
                    neighborCount[fInd[i][2]] += 1;                                

                }

                for(size_t i=0; i<normals.size(); i++)
                {
                    normals[i] /= neighborCount[i];
                }
            }


            // TODO: SONRA BAK

            for(size_t i=0; i<fInd.size(); i++)
            {
                glm::vec3 a,b,c;

                if(fInd[i].size() == 3)
                {
                    a.x = vPos[fInd[i][0]][0];
                    a.y = vPos[fInd[i][0]][1];
                    a.z = vPos[fInd[i][0]][2];

                    b.x = vPos[fInd[i][1]][0];
                    b.y = vPos[fInd[i][1]][1];
                    b.z = vPos[fInd[i][1]][2];                                

                    c.x = vPos[fInd[i][2]][0];
                    c.y = vPos[fInd[i][2]][1];
                    c.z = vPos[fInd[i][2]][2];

                    if(!normalsExist)
                    {
                        Triangle tri(a, b, c, normals[fInd[i][0]], normals[fInd[i][1]], normals[fInd[i][2]]);
                        if(textureCoordsExist)
                        {
                            tri.texCoordA = glm::vec2(u[fInd[i][0]], v[fInd[i][0]]);
                            tri.texCoordB = glm::vec2(u[fInd[i][1]], v[fInd[i][1]]);
                            tri.texCoordC = glm::vec2(u[fInd[i][2]], v[fInd[i][2]]);                                                        
                        }
                        triangleList.push_back(tri);                        
                    }
                    else
                    {
                        Triangle tri(a, b, c, glm::vec3(nx[fInd[i][0]], ny[fInd[i][0]], nz[fInd[i][0]]),
                                              glm::vec3(nx[fInd[i][1]], ny[fInd[i][1]], nz[fInd[i][1]]),
                                              glm::vec3(nx[fInd[i][2]], ny[fInd[i][2]], nz[fInd[i][2]]));
                        if(textureCoordsExist)
                        {
                            tri.texCoordA = glm::vec2(u[fInd[i][0]], v[fInd[i][0]]);
                            tri.texCoordB = glm::vec2(u[fInd[i][1]], v[fInd[i][1]]);
                            tri.texCoordC = glm::vec2(u[fInd[i][2]], v[fInd[i][2]]);                                                        
                        }
                        triangleList.push_back(tri);                        
                    }

                    

                }
                else if(fInd[i].size() == 4)
                {
                    a.x = vPos[fInd[i][0]][0];
                    a.y = vPos[fInd[i][0]][1];
                    a.z = vPos[fInd[i][0]][2];

                    b.x = vPos[fInd[i][1]][0];
                    b.y = vPos[fInd[i][1]][1];
                    b.z = vPos[fInd[i][1]][2];                                

                    c.x = vPos[fInd[i][2]][0];
                    c.y = vPos[fInd[i][2]][1];
                    c.z = vPos[fInd[i][2]][2];

                    if(!normalsExist)
                    {
                        Triangle tri(a, b, c, normals[fInd[i][0]], normals[fInd[i][1]], normals[fInd[i][2]]);
                        if(textureCoordsExist)
                        {
                            tri.texCoordA = glm::vec2(u[fInd[i][0]], v[fInd[i][0]]);
                            tri.texCoordB = glm::vec2(u[fInd[i][1]], v[fInd[i][1]]);
                            tri.texCoordC = glm::vec2(u[fInd[i][2]], v[fInd[i][2]]);                                                        
                        }
                        triangleList.push_back(tri);                        
                    }
                    else
                    {
                        Triangle tri(a, b, c, glm::vec3(nx[fInd[i][0]], ny[fInd[i][0]], nz[fInd[i][0]]),
                                              glm::vec3(nx[fInd[i][1]], ny[fInd[i][1]], nz[fInd[i][1]]),
                                              glm::vec3(nx[fInd[i][2]], ny[fInd[i][2]], nz[fInd[i][2]]));
                        if(textureCoordsExist)
                        {
                            tri.texCoordA = glm::vec2(u[fInd[i][0]], v[fInd[i][0]]);
                            tri.texCoordB = glm::vec2(u[fInd[i][1]], v[fInd[i][1]]);
                            tri.texCoordC = glm::vec2(u[fInd[i][2]], v[fInd[i][2]]);                                                        
                        }
                        triangleList.push_back(tri);                        
                    }                    

                    b.x = vPos[fInd[i][2]][0];
                    b.y = vPos[fInd[i][2]][1];
                    b.z = vPos[fInd[i][2]][2];                                

                    c.x = vPos[fInd[i][3]][0];
                    c.y = vPos[fInd[i][3]][1];
                    c.z = vPos[fInd[i][3]][2];

                    if(!normalsExist)
                    {
                        Triangle tri2(a, b, c, normals[fInd[i][0]], normals[fInd[i][2]], normals[fInd[i][3]]);
                        if(textureCoordsExist)
                        {
                            tri2.texCoordA = glm::vec2(u[fInd[i][0]], v[fInd[i][0]]);
                            tri2.texCoordB = glm::vec2(u[fInd[i][2]], v[fInd[i][2]]);
                            tri2.texCoordC = glm::vec2(u[fInd[i][3]], v[fInd[i][3]]);                                                        
                        }
                        triangleList.push_back(tri2);                        
                    }
                    else
                    {
                        Triangle tri2(a, b, c, glm::vec3(nx[fInd[i][0]], ny[fInd[i][0]], nz[fInd[i][0]]),
                                               glm::vec3(nx[fInd[i][2]], ny[fInd[i][2]], nz[fInd[i][2]]),
                                               glm::vec3(nx[fInd[i][3]], ny[fInd[i][3]], nz[fInd[i][3]]));
                        if(textureCoordsExist)
                        {
                            tri2.texCoordA = glm::vec2(u[fInd[i][0]], v[fInd[i][0]]);
                            tri2.texCoordB = glm::vec2(u[fInd[i][2]], v[fInd[i][2]]);
                            tri2.texCoordC = glm::vec2(u[fInd[i][3]], v[fInd[i][3]]);                                                        
                        }
                        triangleList.push_back(tri2);                        
                    }

                }              
                
            }

        }
        else
        {
            Indices indices;

            std::vector<Indices> indexVector;

            int vertexOffset = 0;
            int textureOffset = 0;
            
            if(child->Attribute("vertexOffset"))
                vertexOffset = std::stoi(child->Attribute("vertexOffset"));
            if(child->Attribute("textureOffset"))
                textureOffset = std::stoi(child->Attribute("textureOffset"));

            for(size_t i=0; i<_vertexData.size(); i++)
            {
                normals.push_back(glm::vec3(0.0));
                neighborCount.push_back(0);
            }


            while(!(stream >> indices.a).eof())
            {
                stream >> indices.b >> indices.c;

                indexVector.push_back(indices);

                glm::vec3 a = _vertexData[indices.a + vertexOffset - 1];
                glm::vec3 b = _vertexData[indices.b + vertexOffset - 1];
                glm::vec3 c = _vertexData[indices.c + vertexOffset - 1];            
                

                glm::vec3 normal = glm::normalize(glm::cross((b-a), (c-a)));

                normals[indices.a + vertexOffset - 1] += normal;
                normals[indices.b + vertexOffset - 1] += normal;
                normals[indices.c + vertexOffset - 1] += normal;

                neighborCount[indices.a + vertexOffset - 1] += 1;
                neighborCount[indices.b + vertexOffset - 1] += 1;
                neighborCount[indices.c + vertexOffset - 1] += 1;

            }

            for(size_t i=0; i<normals.size(); i++)
            {
                normals[i] /= neighborCount[i];
            }


            for(size_t i=0; i<indexVector.size(); i++)
            {

                glm::vec3 a = _vertexData[indexVector[i].a + vertexOffset - 1];
                glm::vec3 b = _vertexData[indexVector[i].b + vertexOffset - 1];                
                glm::vec3 c = _vertexData[indexVector[i].c + vertexOffset - 1];                

                Triangle tri(a, b, c, normals[indexVector[i].a + vertexOffset - 1], normals[indexVector[i].b + vertexOffset - 1], normals[indexVector[i].c + vertexOffset - 1]);

                if(_texCoordData.size() > indexVector[i].a + textureOffset - 1)
                    tri.texCoordA = _texCoordData[indexVector[i].a + textureOffset - 1];
                if(_texCoordData.size() > indexVector[i].b + textureOffset - 1)
                    tri.texCoordB = _texCoordData[indexVector[i].b + textureOffset - 1];
                if(_texCoordData.size() > indexVector[i].c + textureOffset - 1)
                    tri.texCoordC = _texCoordData[indexVector[i].c + textureOffset - 1];

                triangleList.push_back(tri);
            }
        }
        stream.clear();
        
        Mesh m(triangleList, materialId - 1, softShading);

        child = element->FirstChildElement("Textures");
        if(child)
        {
            stream << child->GetText() << std::endl;
            int texIndex;
            while(!(stream >> texIndex).eof())
            {
                if(_textures[texIndex - 1]->mapType == MapType::BUMP_MAP)
                {
                    m.bumpMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::DIFFUSE_MAP)
                {
                    m.diffuseMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::EMISSION_MAP)
                {
                    m.emissionMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::NORMAL_MAP)
                {
                    m.normalMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::ROUGHNESS_MAP)
                {
                    m.roughnessMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::SPECULAR_MAP)
                {
                    m.specularMap = _textures[texIndex - 1];
                }
            }
        }
        stream.clear();

        child = element->FirstChildElement("MotionBlur");
        glm::vec3 motionBlurTranslationVector(0.0f);
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> motionBlurTranslationVector.x >> motionBlurTranslationVector.y >> motionBlurTranslationVector.z;
        }

        m.translationVector = motionBlurTranslationVector;


        child = element->FirstChildElement("Transformations");
        glm::mat4 model(1.0f);        
        if(child)
        {
            std::vector<std::string> transformations = split(std::string(child->GetText()));
            for(size_t i=0; i<transformations.size(); i++)
            {
                char type = transformations[i][0];
                std::string::iterator it = transformations[i].begin();
                it++;
                std::string idString(it, transformations[i].end());
                std::stringstream strToInt(idString);
                int id = 0;
                strToInt >> id;

                if(type == 'r')
                {
                    model =  _rotationMatrices[id - 1] * model;
                }
                else if(type == 't')
                {
                    model = _translationMatrices[id - 1] * model;
                }
                else if(type == 's')
                {
                    model = _scalingMatrices[id - 1] * model;
                }
                else if(type == 'c')
                {
                    model = _compositeMatrices[id - 1] * model;
                }
            }
        }
        m.transformationMatrix = model;
        m.transformationMatrixInversed = glm::inverse(model);
        m.transformationMatrixInverseTransposed = glm::transpose(m.transformationMatrixInversed);
        _meshes.push_back(m);

        stream.clear();
        element = element->NextSiblingElement("Mesh");
    }
    stream.clear();
}

inline void SceneReadMeshInstances(tinyxml2::XMLNode* root, std::vector<Mesh>& _meshes, std::vector<Texture*>& _textures, std::vector<MeshInstance>& _meshInstances, std::vector<glm::mat4>& _rotationMatrices, std::vector<glm::mat4>& _scalingMatrices, std::vector<glm::mat4>& _translationMatrices, std::vector<glm::mat4>& _compositeMatrices)
{
    std::stringstream stream;
    // Get Meshes
    auto element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("MeshInstance");


    while(element)
    {
        int materialId;
        auto child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> materialId;

        int meshId;
        std::string resetTransform;
        MeshInstance meshInstance;
        meshInstance.materialId = materialId - 1;

        child = element->FirstChildElement("MotionBlur");
        glm::vec3 motionBlurTranslationVector(0.0f);
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> motionBlurTranslationVector.x >> motionBlurTranslationVector.y >> motionBlurTranslationVector.z;
        }

        child = element->FirstChildElement("Textures");
        if(child)
        {
            stream << child->GetText() << std::endl;
            int texIndex;
            while(!(stream >> texIndex).eof())
            {
                if(_textures[texIndex - 1]->mapType == MapType::BUMP_MAP)
                {
                    meshInstance.bumpMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::DIFFUSE_MAP)
                {
                    meshInstance.diffuseMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::EMISSION_MAP)
                {
                    meshInstance.emissionMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::NORMAL_MAP)
                {
                    meshInstance.normalMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::ROUGHNESS_MAP)
                {
                    meshInstance.roughnessMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::SPECULAR_MAP)
                {
                    meshInstance.specularMap = _textures[texIndex - 1];
                }
            }
        }
        stream.clear();        

        meshInstance.translationVector = motionBlurTranslationVector;

        child = element->FirstChildElement("Transformations");
        glm::mat4 model(1.0f);        
        if(child)
        {
            std::vector<std::string> transformations = split(std::string(child->GetText()));
            for(size_t i=0; i<transformations.size(); i++)
            {
                char type = transformations[i][0];
                std::string::iterator it = transformations[i].begin();
                it++;
                std::string idString(it, transformations[i].end());
                std::stringstream strToInt(idString);
                int id = 0;
                strToInt >> id;

                if(type == 'r')
                {
                    model =  _rotationMatrices[id - 1] * model;
                }
                else if(type == 't')
                {
                    model = _translationMatrices[id - 1] * model;
                }
                else if(type == 's')
                {
                    model = _scalingMatrices[id - 1] * model;
                }
                else if(type == 'c')
                {
                    model = _compositeMatrices[id - 1] * model;
                }
            }
        }     

        if(element->Attribute("baseMeshId") && element->Attribute("resetTransform"))
        {
            stream << element->Attribute("baseMeshId");
            stream >> meshId;

            meshInstance.mesh = &_meshes[meshId - 1];

            resetTransform = element->Attribute("resetTransform");
            
            if(resetTransform == "true")
            {
                meshInstance.transformationMatrix = model;
                meshInstance.transformationMatrixInversed = glm::inverse(model);
                meshInstance.transformationMatrixInverseTransposed = glm::transpose(meshInstance.transformationMatrixInversed);
            }
            else if(resetTransform == "false")
            {
                model = model * meshInstance.mesh->transformationMatrix;
                meshInstance.transformationMatrix = model;
                meshInstance.transformationMatrixInversed = glm::inverse(model);
                meshInstance.transformationMatrixInverseTransposed = glm::transpose(meshInstance.transformationMatrixInversed);

            }

            _meshInstances.push_back(meshInstance);
        }

        element = element->NextSiblingElement("MeshInstance");


        stream.clear();
    }
    stream.clear();
}


inline void SceneReadSpheres(tinyxml2::XMLNode* root, std::vector<Sphere>& _spheres, std::vector<Texture*>& _textures, std::vector<glm::vec3>& _vertexData, std::vector<glm::mat4>& _rotationMatrices, std::vector<glm::mat4>& _scalingMatrices, std::vector<glm::mat4>& _translationMatrices, std::vector<glm::mat4>& _compositeMatrices)
{
    std::stringstream stream;
    // Get Spheres
    auto element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Sphere");

    size_t materialId;
    size_t centerVertexId;
    float radius;

    while(element)
    {
        auto child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> materialId;

        child = element->FirstChildElement("Center");
        stream << child->GetText() << std::endl;
        stream >> centerVertexId;

        child = element->FirstChildElement("Radius");
        stream << child->GetText() << std::endl;
        stream >> radius;

        glm::vec3 centerVertex = _vertexData[centerVertexId - 1];

        Sphere sphere(centerVertex, radius, materialId - 1);

        child = element->FirstChildElement("Textures");
        if(child)
        {
            stream << child->GetText() << std::endl;
            int texIndex;
            while(!(stream >> texIndex).eof())
            {
                if(_textures[texIndex - 1]->mapType == MapType::BUMP_MAP)
                {
                    sphere.bumpMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::DIFFUSE_MAP)
                {
                    sphere.diffuseMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::EMISSION_MAP)
                {
                    sphere.emissionMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::NORMAL_MAP)
                {
                    sphere.normalMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::ROUGHNESS_MAP)
                {
                    sphere.roughnessMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::SPECULAR_MAP)
                {
                    sphere.specularMap = _textures[texIndex - 1];
                }
            }
        }
        stream.clear();          

        child = element->FirstChildElement("MotionBlur");
        glm::vec3 motionBlurTranslationVector(0.0f);
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> motionBlurTranslationVector.x >> motionBlurTranslationVector.y >> motionBlurTranslationVector.z;
        }

        sphere.translationVector = motionBlurTranslationVector;


        child = element->FirstChildElement("Transformations");
        glm::mat4 model(1.0f);        
        if(child)
        {
            std::vector<std::string> transformations = split(std::string(child->GetText()));
            for(size_t i=0; i<transformations.size(); i++)
            {
                char type = transformations[i][0];
                std::string::iterator it = transformations[i].begin();
                it++;
                std::string idString(it, transformations[i].end());
                std::stringstream strToInt(idString);
                int id = 0;
                strToInt >> id;

                if(type == 'r')
                {
                    model =  _rotationMatrices[id - 1] * model;
                }
                else if(type == 't')
                {
                    model = _translationMatrices[id - 1] * model;
                }
                else if(type == 's')
                {
                    model = _scalingMatrices[id - 1] * model;
                }
                else if(type == 'c')
                {
                    model = _compositeMatrices[id -1] * model;
                }
            }
        }
        sphere.transformationMatrix = model;
        sphere.transformationMatrixInversed = glm::inverse(model);
        sphere.transformationMatrixInverseTransposed = glm::transpose(sphere.transformationMatrixInversed);
        _spheres.push_back(sphere);
        element = element->NextSiblingElement("Sphere");
    }
    stream.clear();
}

inline void SceneReadTriangles(tinyxml2::XMLNode* root, std::vector<Triangle>& _triangles, std::vector<Texture*>& _textures, std::vector<glm::vec3>& _vertexData, std::vector<glm::vec2>& _texCoordData, std::vector<glm::mat4>& _rotationMatrices, std::vector<glm::mat4>& _scalingMatrices, std::vector<glm::mat4>& _translationMatrices, std::vector<glm::mat4>& _compositeMatrices)
{
    std::stringstream stream;
    // Get Triangles
    auto element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Triangle");
    Indices indices;
    size_t materialId;

    while(element)
    {
        auto child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> materialId;

        child = element->FirstChildElement("Indices");

        int vertexOffset = 0;
        int textureOffset = 0;
            
        if(child->Attribute("vertexOffset"))
            vertexOffset = std::stoi(child->Attribute("vertexOffset"));
        if(child->Attribute("textureOffset"))
            textureOffset = std::stoi(child->Attribute("textureOffset"));        

        stream << child->GetText() << std::endl;
        stream >> indices.a >> indices.b >> indices.c;
        
        glm::vec3 a = _vertexData[indices.a + vertexOffset - 1];
        glm::vec3 b = _vertexData[indices.b + vertexOffset - 1];
        glm::vec3 c = _vertexData[indices.c + vertexOffset - 1];

        Triangle tri(a, b, c);
        if(_texCoordData.size() > indices.a + textureOffset - 1)
            tri.texCoordA = _texCoordData[indices.a + textureOffset - 1];
        if(_texCoordData.size() > indices.b + textureOffset - 1)
            tri.texCoordB = _texCoordData[indices.b + textureOffset - 1];
        if(_texCoordData.size() > indices.c + textureOffset - 1)
            tri.texCoordC = _texCoordData[indices.c + textureOffset - 1];

        tri.materialId = materialId - 1;

        child = element->FirstChildElement("Textures");
        if(child)
        {
            stream << child->GetText() << std::endl;
            int texIndex;
            while(!(stream >> texIndex).eof())
            {
                if(_textures[texIndex - 1]->mapType == MapType::BUMP_MAP)
                {
                    tri.bumpMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::DIFFUSE_MAP)
                {
                    tri.diffuseMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::EMISSION_MAP)
                {
                    tri.emissionMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::NORMAL_MAP)
                {
                    tri.normalMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::ROUGHNESS_MAP)
                {
                    tri.roughnessMap = _textures[texIndex - 1];
                }
                else if(_textures[texIndex - 1]->mapType == MapType::SPECULAR_MAP)
                {
                    tri.specularMap = _textures[texIndex - 1];
                }
            }
        }
        stream.clear(); 

        child = element->FirstChildElement("MotionBlur");
        glm::vec3 motionBlurTranslationVector(0.0f);
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> motionBlurTranslationVector.x >> motionBlurTranslationVector.y >> motionBlurTranslationVector.z;
        }

        tri.translationVector = motionBlurTranslationVector;

        child = element->FirstChildElement("Transformations");
        glm::mat4 model(1.0f);        
        if(child)
        {
            std::vector<std::string> transformations = split(std::string(child->GetText()));
            for(size_t i=0; i<transformations.size(); i++)
            {
                char type = transformations[i][0];
                std::string::iterator it = transformations[i].begin();
                it++;
                std::string idString(it, transformations[i].end());
                std::stringstream strToInt(idString);
                int id = 0;
                strToInt >> id;

                if(type == 'r')
                {
                    model =  _rotationMatrices[id - 1] * model;
                }
                else if(type == 't')
                {
                    model = _translationMatrices[id - 1] * model;
                }
                else if(type == 's')
                {
                    model = _scalingMatrices[id - 1] * model;
                }
                else if(type == 'c')
                {
                    model = _compositeMatrices[id - 1] * model;
                }                
            }
        }
        tri.transformationMatrix = model;
        tri.transformationMatrixInversed = glm::inverse(model);
        tri.transformationMatrixInverseTransposed = glm::transpose(tri.transformationMatrixInversed);
        _triangles.push_back(tri);
        element = element->NextSiblingElement("Triangle");
    }
}


inline void SceneReadTranslations(tinyxml2::XMLElement* element, std::vector<glm::mat4>& _translationMatrices)
{
    std::stringstream stream;
    auto translation = element->FirstChildElement("Translation");

    while(translation)
    {
        stream << translation->GetText();
        float x,y,z;
        stream >> x >> y >> z;
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        _translationMatrices.push_back(translationMatrix);

        translation = translation->NextSiblingElement("Translation");
        stream.clear();
    }

}

inline void SceneReadRotations(tinyxml2::XMLElement* element, std::vector<glm::mat4>& _rotationMatrices)
{
    std::stringstream stream;
    auto rotation = element->FirstChildElement("Rotation");

    while(rotation)
    {
        stream << rotation->GetText();
        float angle, x, y, z;
        stream >> angle >> x >> y >> z;
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(x, y, z));
        _rotationMatrices.push_back(rotationMatrix);

        rotation = rotation->NextSiblingElement("Rotation");
        stream.clear();        
    }

}

inline void SceneReadScalings(tinyxml2::XMLElement* element, std::vector<glm::mat4>& _scalingMatrices)
{
    std::stringstream stream;
    auto scaling = element->FirstChildElement("Scaling");

    while(scaling)
    {
        stream << scaling->GetText();
        float x, y, z;
        stream >> x >> y >> z;
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z));
        _scalingMatrices.push_back(scalingMatrix);

        scaling = scaling->NextSiblingElement("Scaling");
        stream.clear();        
    }

}

inline void SceneReadComposite(tinyxml2::XMLElement* element, std::vector<glm::mat4>& _compositeMatrices)
{
    std::stringstream stream;
    auto composite = element->FirstChildElement("Composite");

    while(composite)
    {
        glm::vec4 v1,v2,v3,v4;

        stream << composite->GetText();

        stream >> v1.x >> v2.x >> v3.x >> v4.x >>
                  v1.y >> v2.y >> v3.y >> v4.y >>
                  v1.z >> v2.z >> v3.z >> v4.z >>
                  v1.w >> v2.w >> v3.w >> v4.w;

        glm::mat4 compositeMatrix = glm::mat4(v1,v2,v3,v4);
        _compositeMatrices.push_back(compositeMatrix);

        composite = composite->NextSiblingElement("Composite");
        stream.clear();
    }
}

inline void SceneReadTransformations(tinyxml2::XMLNode* root, std::vector<glm::mat4>& _translationMatrices,
                                                              std::vector<glm::mat4>& _rotationMatrices,
                                                              std::vector<glm::mat4>& _scalingMatrices,
                                                              std::vector<glm::mat4>& _compositeMatrices)
{

    auto element = root->FirstChildElement("Transformations");

    if(element)
    {
        SceneReadTranslations(element, _translationMatrices);
        SceneReadRotations(element, _rotationMatrices);
        SceneReadScalings(element, _scalingMatrices);
        SceneReadComposite(element, _compositeMatrices);
    }

}

inline Ray* RefTransRays(const Ray& ray, const Material& hitMaterial)
{
    Ray result[2];

    return result;
}


inline void ScenePopulateObjects(std::vector<Object*>& _objects, std::vector<Mesh>& _meshes, std::vector<MeshInstance>& _meshInstances, std::vector<Sphere>& _spheres, std::vector<Triangle>& _triangles)
{

    for(size_t i=0; i<_meshes.size(); i++)
    {
        _objects.push_back(&_meshes[i]);
    }

    for(size_t i=0; i<_meshInstances.size(); i++)
    {
        _objects.push_back(&_meshInstances[i]);
    }

    for(size_t i=0; i<_spheres.size(); i++)
    {
        _objects.push_back(&_spheres[i]);
    }

    for(size_t i=0; i<_triangles.size(); i++)
    {
        _objects.push_back(&_triangles[i]);
    }            

}

inline void ScenePopulateLights(std::vector<Light*>& _lights,
                                std::vector<PointLight>& _pointLights,
                                std::vector<AreaLight>& _areaLights,
                                std::vector<DirectionalLight>& _directionalLights)
{
    for(size_t i=0; i<_pointLights.size(); i++)
    {
        _lights.push_back(&_pointLights[i]);
    }
    for(size_t i=0; i<_areaLights.size(); i++)
    {
        _lights.push_back(&_areaLights[i]);
    }
    for(size_t i=0; i<_directionalLights.size(); i++)
    {
        _lights.push_back(&_directionalLights[i]);
    }
}                                

inline float GaussianWeight(float x, float y, float stdDev)
{
    return (1/(2*M_PI*stdDev*stdDev)) * std::pow(EULER, (-1/2)*((x*x + y*y)/(stdDev*stdDev)));
}

inline OrthonormalBasis GiveOrthonormalBasis(glm::vec3 vector)
{

    OrthonormalBasis result;
    glm::vec3 nVec = vector;


    if(std::fabs(nVec.x) <= std::fabs(nVec.y) && std::fabs(nVec.x) <= std::fabs(nVec.z))
    {
        nVec.x = 1.0f;
    }
    else if(std::fabs(nVec.y) <= std::fabs(nVec.x) && std::fabs(nVec.y) <= std::fabs(nVec.z))
    {
        nVec.y = 1.0f;
    }
    else if(std::fabs(nVec.z) <= std::fabs(nVec.x) && std::fabs(nVec.z) <= std::fabs(nVec.y))
    {
        nVec.z = 1.0f;
    }

    result.u = glm::normalize(glm::cross(vector, nVec));
    result.v = glm::normalize(glm::cross(vector, result.u));

    return result;

}

#endif /* __UTILS_H__ */