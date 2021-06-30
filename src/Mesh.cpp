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

    if(std::isnan(report.normal.x))
    {
        std::cout << "asdas" << std::endl;
    }
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
                int u = (report.intersection.x + this->diffuseMap->offset) * this->diffuseMap->scale;
                int v = (report.intersection.z + this->diffuseMap->offset) * this->diffuseMap->scale;
                
                int uPv = u + v;

                // white
                if(uPv % 2 == 0)
                {
                    report.texDiffuseReflectance = this->diffuseMap->blackColor;
                }
                // black
                else
                {
                    report.texDiffuseReflectance = this->diffuseMap->whiteColor;
                }
            } 

            report.diffuseActive = true;
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

            report.specularActive = true;
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
            report.normal = TBN * fetchedNormal;
        }
        else if(this->bumpMap)
        {
            if(this->bumpMap->type != TextureType::PERLIN)
            {
                glm::vec3 e1 = glm::normalize(report.coordB - report.coordA);
                glm::vec3 e2 = glm::normalize(report.coordC - report.coordA);

                glm::mat2 A_Inverse = glm::inverse(glm::mat2(glm::vec2(report.texCoordB.x - report.texCoordA.x, report.texCoordC.x - report.texCoordA.x),
                                                            glm::vec2(report.texCoordB.y - report.texCoordA.y, report.texCoordC.y - report.texCoordA.y)));

                glm::mat3x2 E(glm::vec2(e1.x, e2.x),
                            glm::vec2(e1.y, e2.y),
                            glm::vec2(e1.z, e2.z));

                glm::mat2x3 TB = glm::transpose(A_Inverse * E);

                float u = report.texCoord.x;
                float v = report.texCoord.y;

                if(u < 0 || u > 1)
                    u -= std::floor(u);
                if(v < 0 || v > 1)
                    v -= std::floor(v);


                int i = u * this->bumpMap->image->width;
                int j = v * this->bumpMap->image->height;
                glm::vec3 image = this->bumpMap->image->get(i,j);
                glm::vec3 imageForwardU = this->bumpMap->image->get(i+1, j);
                glm::vec3 imageForwardV = this->bumpMap->image->get(i, j+1);

                float avImage = (image.x + image.y + image.z) / 3;
                float avImageForwardU = (imageForwardU.x + imageForwardU.y + imageForwardU.z) / 3;
                float avImageForwardV = (imageForwardV.x + imageForwardV.y + imageForwardV.z) / 3;

                    
                float du = avImageForwardU - avImage;//glm::length(this->bumpMap->image->get(i+1,j) - this->bumpMap->image->get(i,j));
                float dv = avImageForwardV - avImage;//glm::length(this->bumpMap->image->get(i,j+1) - this->bumpMap->image->get(i,j));

                glm::vec3 tangentPlaneVec = du * TB[0] + dv * TB[1];

                glm::vec3 newNormal = report.normal - this->bumpMap->bumpFactor*(tangentPlaneVec);
                newNormal = glm::normalize(newNormal);
                report.normal = newNormal;

            }
            else
            {
                // perlin gradient
                float epsilon = 0.001;
                float xDiff = this->bumpMap->FetchPerlin(glm::vec3(report.intersection.x + epsilon, report.intersection.y, report.intersection.z));
                float yDiff = this->bumpMap->FetchPerlin(glm::vec3(report.intersection.x, report.intersection.y + epsilon, report.intersection.z));
                float zDiff = this->bumpMap->FetchPerlin(glm::vec3(report.intersection.x, report.intersection.y, report.intersection.z + epsilon));
                float perlinNoise = this->bumpMap->FetchPerlin(report.intersection);

                // Veiny appearence
                if(this->bumpMap->noiseConversion == NoiseConversionType::ABSVAL)
                {
                    xDiff = std::fabs(xDiff);
                    yDiff = std::fabs(yDiff);
                    zDiff = std::fabs(zDiff);
                    perlinNoise = std::fabs(perlinNoise);
                }
                // Patch appearence
                else if(this->bumpMap->noiseConversion == NoiseConversionType::LINEAR)
                {
                    xDiff = (xDiff + 1)/2;
                    yDiff = (yDiff + 1)/2;
                    zDiff = (zDiff + 1)/2;
                    perlinNoise = (perlinNoise + 1)/2;
                }

                glm::vec3 perlinGradient = glm::vec3((xDiff - perlinNoise)/epsilon,
                                                     (yDiff - perlinNoise)/epsilon,
                                                     (zDiff - perlinNoise)/epsilon);

                glm::vec3 projectedGradient = perlinGradient - glm::dot(perlinGradient, report.normal) * report.normal;
                glm::vec3 newNormal = report.normal - this->bumpMap->bumpFactor * (projectedGradient);
                newNormal = glm::normalize(newNormal);
                report.normal = newNormal;
            }
        }

        if(this->emissionMap)
        {
            report.emissionActive = true;
        }

        if(this->roughnessMap)
        {

        }        
    }

    return test;
}