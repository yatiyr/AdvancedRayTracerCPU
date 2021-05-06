#ifndef __UTILS_H__
#define __UTILS_H__

#include <Structures.h>
#include <Mesh.h>
#include <Triangle.h>
#include <Sphere.h>
#include <sstream>
#include <happly.h>
#include <map>


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
        stream << "0.0001" << std::endl;
    }
    stream >> _shadowRayEpsilon;

    element = root->FirstChildElement("IntersectionTestEpsilon");
    if(element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0.0000001" << std::endl;
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

            t = std::tan(fovY/2)*camera.nearDistance;
            r = (camera.imageResolution.x / camera.imageResolution.y) * t;
            l = -r;
            b = -t;

            glm::vec3 gaze = glm::normalize(gazePoint - camera.position);
            camera.gaze = gaze;
            camera.up   = glm::normalize(camera.up);
            camera.v    = glm::normalize(glm::cross(camera.gaze, camera.up));
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
            camera.gaze = glm::normalize(camera.gaze);
            camera.up   = glm::normalize(camera.up);
            camera.v    = glm::normalize(glm::cross(camera.gaze, camera.up));

        }
        
        std::string imageName;
        stream >> imageName;
        imageNames.push_back(imageName);
        _imageName = imageName;

        _cameras.push_back(camera);
        element = element->NextSiblingElement("Camera");
    }
    stream.clear();    
}


inline void SceneReadLights(tinyxml2::XMLNode* root, std::vector<PointLight>& _pointLights, glm::vec3& _ambientLight)
{
    std::stringstream stream;
    // Get Lights
    auto element = root->FirstChildElement("Lights");
    auto child = element->FirstChildElement("AmbientLight");
    stream << child->GetText() << std::endl;
    stream >> _ambientLight.x >> _ambientLight.y >> _ambientLight.z;
    _ambientLight.x /= 255.99;
    _ambientLight.y /= 255.99;
    _ambientLight.z /= 255.99;
    element = element->FirstChildElement("PointLight");
    PointLight pointLight;
    while(element)
    {
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

        stream >> material.ambientReflectance.x >> material.ambientReflectance.y >> material.ambientReflectance.z;
        stream >> material.diffuseReflectance.x >> material.diffuseReflectance.y >> material.diffuseReflectance.z;
        stream >> material.specularReflectance.x >> material.specularReflectance.y >> material.specularReflectance.z;
        stream >> material.mirrorReflectance.x >> material.mirrorReflectance.y >> material.mirrorReflectance.z;
        stream >> material.absorptionCoefficient.x >> material.absorptionCoefficient.y >> material.absorptionCoefficient.z;
        stream >> material.refractionIndex;
        stream >> material.absorptionIndex;
        stream >> material.phongExponent;

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
    stream << element->GetText() << std::endl;
    glm::vec3 vertex;
    while(!(stream >> vertex.x).eof())
    {
        stream >> vertex.y >> vertex.z;
        _vertexData.push_back(vertex);
    }
    stream.clear();
}

inline void SceneReadMeshes(tinyxml2::XMLNode* root, std::vector<Mesh>& _meshes, std::vector<glm::vec3>& _vertexData, std::vector<glm::mat4>& _rotationMatrices, std::vector<glm::mat4>& _scalingMatrices, std::vector<glm::mat4>& _translationMatrices)
{
    std::stringstream stream;
    // Get Meshes
    auto element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Mesh");
    

    while(element)
    {
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
            const char* localPath = child->Attribute("plyFile");
            std::string path = std::string(ROOT_DIR) + "assets/scenes/" + std::string(localPath);

            happly::PLYData plyIn(path);

            std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
            std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

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


            // TODO: SONRA BAK

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

                Triangle tri(a, b, c, normals[fInd[i][0]], normals[fInd[i][1]], normals[fInd[i][2]]);
                triangleList.push_back(tri);
                
            }

        }
        else
        {
            Indices indices;

            std::vector<Indices> indexVector;

            for(size_t i=0; i<_vertexData.size(); i++)
            {
                normals.push_back(glm::vec3(0.0));
                neighborCount.push_back(0);
            }


            while(!(stream >> indices.a).eof())
            {
                stream >> indices.b >> indices.c;

                indexVector.push_back(indices);

                glm::vec3 a = _vertexData[indices.a - 1];
                glm::vec3 b = _vertexData[indices.b - 1];
                glm::vec3 c = _vertexData[indices.c - 1];            
                

                glm::vec3 normal = glm::normalize(glm::cross((b-a), (c-a)));

                normals[indices.a - 1] += normal;
                normals[indices.b - 1] += normal;
                normals[indices.c - 1] += normal;

                neighborCount[indices.a - 1] += 1;
                neighborCount[indices.b - 1] += 1;
                neighborCount[indices.c - 1] += 1;

            }

            for(size_t i=0; i<normals.size(); i++)
            {
                normals[i] /= neighborCount[i];
            }


            for(size_t i=0; i<indexVector.size(); i++)
            {

                glm::vec3 a = _vertexData[indexVector[i].a - 1];
                glm::vec3 b = _vertexData[indexVector[i].b - 1];                
                glm::vec3 c = _vertexData[indexVector[i].c - 1];                

                Triangle tri(a, b, c, normals[indexVector[i].a - 1], normals[indexVector[i].b - 1], normals[indexVector[i].c - 1]);

                triangleList.push_back(tri);
            }
        }

        Mesh m(triangleList, materialId - 1, false);

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


inline void SceneReadSpheres(tinyxml2::XMLNode* root, std::vector<Sphere>& _spheres, std::vector<glm::vec3>& _vertexData, std::vector<glm::mat4>& _rotationMatrices, std::vector<glm::mat4>& _scalingMatrices, std::vector<glm::mat4>& _translationMatrices)
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

inline void SceneReadTriangles(tinyxml2::XMLNode* root, std::vector<Triangle>& _triangles, std::vector<glm::vec3>& _vertexData, std::vector<glm::mat4>& _rotationMatrices, std::vector<glm::mat4>& _scalingMatrices, std::vector<glm::mat4>& _translationMatrices)
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
        stream << child->GetText() << std::endl;
        stream >> indices.a >> indices.b >> indices.c;
        
        glm::vec3 a = _vertexData[indices.a - 1];
        glm::vec3 b = _vertexData[indices.b - 1];
        glm::vec3 c = _vertexData[indices.c - 1];

        Triangle tri(a, b, c);
        tri.materialId = materialId - 1;


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

inline void SceneReadTransformations(tinyxml2::XMLNode* root, std::vector<glm::mat4>& _translationMatrices,
                                                              std::vector<glm::mat4>& _rotationMatrices,
                                                              std::vector<glm::mat4>& _scalingMatrices)
{

    auto element = root->FirstChildElement("Transformations");

    if(element)
    {
        SceneReadTranslations(element, _translationMatrices);
        SceneReadRotations(element, _rotationMatrices);
        SceneReadScalings(element, _scalingMatrices);
    }

}

inline Ray* RefTransRays(const Ray& ray, const Material& hitMaterial)
{
    Ray result[2];

    return result;
}



#endif /* __UTILS_H__ */