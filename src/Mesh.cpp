#include <Mesh.h>


Mesh::Mesh(const std::vector<Triangle>& triangleList, size_t materialId, bool softShadingFlag)
{
    this->bvhRoot = new BVH(triangleList, 0, 200, 0);
    this->materialId = materialId;
    this->softShadingFlag = softShadingFlag;
}

bool Mesh::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling)
{

    glm::mat4 motionBlurTranslationMatrix = MotionBlurTranslate(ray.time);
    glm::mat4 nTMI = transformationMatrixInversed * motionBlurTranslationMatrix;
    glm::mat4 nTMIT = glm::transpose(motionBlurTranslationMatrix) * transformationMatrixInverseTransposed;


    glm::vec3 newOrigin = (nTMI * glm::vec4(ray.origin, 1.0f));
    glm::vec3 newDirection = (nTMI * glm::vec4(ray.direction, 0.0f));
    Ray newRay(newOrigin, newDirection);

    bool test = bvhRoot->Intersect(newRay, report, tmin, tmax, intersectionEpsilon, softShadingFlag, nTMIT, backfaceCulling);

    report.materialId = materialId;
    if(test)
    {
        report.intersection = ray.origin + ray.direction * report.d;

        if(this->diffuseMap)
        {
            report.replaceAll = false;
            if(this->diffuseMap->decalMode == DecalMode::REPLACE_ALL)
                report.replaceAll = true;
            else if(this->diffuseMap->decalMode == DecalMode::REPLACE_KD)
                report.texDiffuseKdMode = 1;
            else if(this->diffuseMap->decalMode == DecalMode::BLEND_KD)
                report.texDiffuseKdMode = 2;

            if(this->diffuseMap->type == TextureType::IMAGE)
            {
                report.texDiffuseReflectance = this->diffuseMap->Fetch(report.texCoord.x, report.texCoord.y);
            }
            else if(this->diffuseMap->type == TextureType::PERLIN)
            {

                float perlin = this->diffuseMap->FetchPerlin(report.intersection);
                // Veiny appearence
                if(this->diffuseMap->noiseConversion == NoiseConversionType::ABSVAL)
                {
                    report.texDiffuseReflectance = glm::vec3(std::fabs(perlin));
                }
                // Patch appearence
                else if(this->diffuseMap->noiseConversion == NoiseConversionType::LINEAR)
                {
                    report.texDiffuseReflectance = glm::vec3((perlin + 1)/2);
                }
            }
            else if(this->diffuseMap->type == TextureType::CHECKERBOARD)
            {
                // Will be implemented
            } 
        }
        
        if(this->specularMap)
        {

            if(this->specularMap->decalMode == DecalMode::REPLACE_KD)
                report.texSpecularKdMode = 1;
            else if(this->specularMap->decalMode == DecalMode::BLEND_KD)
                report.texSpecularKdMode = 2;

            if(this->specularMap->type == TextureType::IMAGE)
            {
                report.texSpecularReflectance = this->specularMap->Fetch(report.texCoord.x, report.texCoord.y);
            }
            else if(this->diffuseMap->type == TextureType::PERLIN)
            {

                float perlin = this->diffuseMap->FetchPerlin(report.intersection);
                // Veiny appearence
                if(this->diffuseMap->noiseConversion == NoiseConversionType::ABSVAL)
                {
                    report.texDiffuseReflectance = glm::vec3(std::fabs(perlin));
                }
                // Patch appearence
                else if(this->diffuseMap->noiseConversion == NoiseConversionType::LINEAR)
                {
                    report.texDiffuseReflectance = glm::vec3((perlin + 1)/2);
                }
            }
            else if(this->diffuseMap->type == TextureType::CHECKERBOARD)
            {
                // Will be implemented
            } 
        }

        if(this->normalMap)
        {
            glm::vec3 fetchedNormal = this->normalMap->Fetch(report.texCoord.x, report.texCoord.y);
            fetchedNormal.x /= 255;
            fetchedNormal.y /= 255;
            fetchedNormal.z /= 255;

            fetchedNormal -= glm::vec3(0.5, 0.5, 0.5);
            fetchedNormal = glm::normalize(fetchedNormal);

            glm::vec3 e1 = report.coordB - report.coordA;
            glm::vec3 e2 = report.coordC - report.coordA;

            glm::mat2 A_Inverse = glm::inverse(glm::mat2(glm::vec2(report.texCoordB.x - report.texCoordA.x, report.texCoordC.x - report.texCoordA.x),
                                                         glm::vec2(report.texCoordB.y - report.texCoordA.y, report.texCoordC.y - report.texCoordA.y)));

            glm::mat3x2 E(glm::vec2(e1.x, e2.x),
                          glm::vec2(e1.y, e2.y),
                          glm::vec2(e1.z, e2.z));

            glm::mat2x3 TB = glm::transpose(A_Inverse * E);
            glm::mat3 TBN(TB[0], TB[1], report.normal);
            report.normal = TBN * report.normal;
        }
        else if(this->bumpMap)
        {

        }

        if(this->emissionMap)
        {

        }

        if(this->roughnessMap)
        {

        }        
    }

    return test;
}