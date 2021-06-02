#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <Image.h>
#include <glm/glm.hpp>
#include <math.h>
#include <algorithm>
#include <PerlinNoiseGenerator.h>

enum MapType
{
    DIFFUSE_MAP   = 0,
    SPECULAR_MAP  = 1,
    EMISSION_MAP  = 2,
    NORMAL_MAP    = 3,
    BUMP_MAP      = 4,
    ROUGHNESS_MAP = 5
};

enum InterpolationType
{
    NEAREST_NEIGHBOR = 0,
    BILINEAR         = 1
};

enum DecalMode
{
    REPLACE_BACKGROUND = 0,
    REPLACE_KD         = 1,
    BLEND_KD           = 2,
    REPLACE_ALL        = 4,
    REPLACE_NORMAL     = 8,
    BUMP_NORMAL        = 16
};

enum TextureType
{
    IMAGE  = 0,
    PERLIN = 1,
    CHECKERBOARD = 2
};

enum NoiseConversionType
{
    ABSVAL = 0,
    LINEAR = 1
};

// TODO: I will add perlin noise soon
class Texture 
{
public:

    // Texture
    MapType mapType;
    TextureType type;
    InterpolationType interpolationType;
    DecalMode decalMode;

    float bumpFactor;

    // Image Texture
    Image* image;    
    int normalizer;


    // Perlin Texture
    PerlinNoiseGenerator noiseGenerator;
    NoiseConversionType noiseConversion;
    float noiseScale;

    // Checkerboard Texture
    glm::vec3 blackColor;
    glm::vec3 whiteColor;
    float scale;
    float offset;

    Texture() {}

    Texture(const char* filename, InterpolationType interpolationType, DecalMode decalMode, TextureType type)
    {
        this->image             = new Image(filename);
        this->interpolationType = interpolationType;
        this->decalMode         = decalMode;
        this->type              = type;
    }

    Texture(Image *image, InterpolationType interpolationType, DecalMode decalMode, TextureType type)
    {
        this->image             = image;
        this->interpolationType = interpolationType;
        this->decalMode         = decalMode;
        this->type              = type;
    }

    Texture(Image *image, InterpolationType interpolationType, DecalMode decalMode, TextureType type, float bumpFactor)
    {
        this->image             = image;
        this->interpolationType = interpolationType;
        this->decalMode         = decalMode;
        this->type              = type;
        this->bumpFactor        = bumpFactor;
    }      

    Texture(InterpolationType interpolationType, DecalMode decalMode, TextureType type, NoiseConversionType noiseConversion, float noiseScale)
    {
        this->interpolationType = interpolationType;
        this->decalMode         = decalMode;
        this->type              = type;
        this->noiseConversion   = noiseConversion;
        this->noiseScale        = noiseScale;
    }

    ~Texture()
    {
        if(image != nullptr)
            delete image;
    }

    void BindImage(Image *image)
    {
        this->image = image;
    }

    float FetchPerlin(glm::vec3 point)
    {
        glm::vec3 scaledPoint = point * noiseScale;
        return noiseGenerator.GenerateNoise(scaledPoint);
    }

    glm::vec3 Fetch(float u, float v)
    {

        if(type == TextureType::IMAGE)
        {
            if(u < 0 || u > 1)
                u -= std::floor(u);
            if(v < 0 || v > 1)
                v -= std::floor(v);

            float i = u * image->width;
            float j = v * image->height;

            if(interpolationType == InterpolationType::NEAREST_NEIGHBOR)
            {
                int iI = std::round(i);
                int iJ = std::round(j);

                glm::vec3 result = image->get(iI, iJ);

                result.x /= normalizer;
                result.y /= normalizer;
                result.z /= normalizer;

                return result;
            }
            else if(interpolationType == InterpolationType::BILINEAR)
            {

                int iI = std::floor(i);
                int iJ = std::floor(j);            

                float a = i - iI;
                float b = j - iJ;           

                glm::vec3 result = (1-a) * (1-b) * image->get(iI, iJ) +
                                   (1-a) *   b   * image->get(iI, iJ + 1) +
                                    a    * (1-b) * image->get(iI + 1, iJ) +
                                    a    *   b   * image->get(iI + 1, iJ + 1);

                result.x /= normalizer;
                result.y /= normalizer;
                result.z /= normalizer;

                return result;
            }
        }

    }
};


#endif