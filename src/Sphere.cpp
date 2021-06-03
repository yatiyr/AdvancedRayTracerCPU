#include <Sphere.h>

Sphere::Sphere(glm::vec3 center, float radius, size_t materialId)
{
    this->center = center;
    this->radius = radius;
    this->materialId = materialId;
}

bool Sphere::solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discriminant = b*b -4*a*c;
    if(discriminant < 0)
        return false;
    else if(discriminant == 0)
        x0 = x1 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discriminant)) :
            -0.5 * (b - sqrt(discriminant));
        x0 = q / a;
        x1 = c / q;
    }

    if (x0 > x1)
        std::swap(x0, x1);

    return true;
        
}

bool Sphere::Intersect(const Ray& r, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling)
{

    glm::mat4 motionBlurTranslationMatrix = MotionBlurTranslate(r.time);

    glm::vec3 newOrigin = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(r.origin, 1.0f));
    glm::vec3 newDirection = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(r.direction, 0.0f));
    Ray newRay(newOrigin, newDirection);

    report.intersection = glm::vec3(-FLT_MAX);
    report.d            = FLT_MAX;

    float discriminant = pow(glm::dot(newRay.direction, (newRay.origin - center)), 2) -
                         dot(newRay.direction, newRay.direction) * (glm::dot(newRay.origin - center, newRay.origin - center) -
                         radius * radius);

    float t;

    if(discriminant >= 0)
    {
        float t1 = -(glm::dot(newRay.direction, (newRay.origin - center)) + sqrt(discriminant)) / glm::dot(newRay.direction, newRay.direction);
        float t2 = -(glm::dot(newRay.direction, (newRay.origin - center)) - sqrt(discriminant)) / glm::dot(newRay.direction, newRay.direction);

        t = std::min(t1, t2);

        if(t1 * t2 < 0)
            t = std::max(t1, t2);

        if(t > tmin - intersectionEpsilon && t < tmax + intersectionEpsilon)
        {
            report.d            = t;
            report.intersection = r.origin + t*r.direction;
            report.materialId   = materialId;
            glm::vec3 normal = (newRay.origin + t*newRay.direction) - center;
            report.normal       = glm::transpose(motionBlurTranslationMatrix)*transformationMatrixInverseTransposed * glm::vec4(normal, 0.0f);
            report.normal = glm::normalize(report.normal);

            glm::vec3 localIntersection = newRay.origin + t*newRay.direction;

            float x = localIntersection.x - this->center.x;
            float y = localIntersection.y - this->center.y;
            float z = localIntersection.z - this->center.z;

            float theta = std::acos(y/this->radius);
            float phi   = std::atan2(z, x);

            report.texCoord.x = (-phi + M_PI) / (2 * M_PI);
            report.texCoord.y = theta / M_PI;


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

                glm::vec3 tangent = glm::vec3(z * 2 * M_PI,
                                              0,
                                              -x * 2 * M_PI);
                tangent = glm::normalize(tangent);

                glm::vec3 bitangent = glm::vec3(y * std::cos(phi) * M_PI,
                                                -this->radius * std::sin(theta) * M_PI,
                                                y * std::sin(phi) * M_PI);
                bitangent = glm::normalize(bitangent);

                glm::mat3 TBN(tangent, bitangent, report.normal);
                report.normal = TBN * fetchedNormal;
            }
            else if(this->bumpMap)
            {
                if(this->bumpMap->type != TextureType::PERLIN)
                {
                    glm::vec3 tangent = glm::vec3(z * 2 * M_PI,
                                                0,
                                                -x * 2 * M_PI);
                    tangent = tangent;

                    glm::vec3 bitangent = glm::vec3(y * std::cos(phi) * M_PI,
                                                    -this->radius * std::sin(theta) * M_PI,
                                                    y * std::sin(phi) * M_PI);
                    bitangent = bitangent;

                    int i = report.texCoord.x * this->bumpMap->image->width;
                    int j = report.texCoord.y * this->bumpMap->image->height;

                    glm::vec3 image = this->bumpMap->image->get(i,j);
                    glm::vec3 imageForwardU = this->bumpMap->image->get(i+1, j);
                    glm::vec3 imageForwardV = this->bumpMap->image->get(i, j+1);

                    float avImage = (image.x + image.y + image.z) / 3;
                    float avImageForwardU = (imageForwardU.x + imageForwardU.y + imageForwardU.z) / 3;
                    float avImageForwardV = (imageForwardV.x + imageForwardV.y + imageForwardV.z) / 3;

                    avImage /= 255;
                    avImageForwardU /= 255;
                    avImageForwardV /= 255;
                    
                    float du = avImageForwardU - avImage;//glm::length(this->bumpMap->image->get(i+1,j) - this->bumpMap->image->get(i,j));
                    float dv = avImageForwardV - avImage;//glm::length(this->bumpMap->image->get(i,j+1) - this->bumpMap->image->get(i,j));

                    glm::vec3 tangentPlaneVec = du * tangent + dv * bitangent;
                    //tangentPlaneVec = glm::normalize(tangentPlaneVec);

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

            return true;
        }
      
    }

    return false;

}