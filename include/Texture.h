#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <Image.h>
#include <glm/glm.hpp>
#include <math.h>
#include <algorithm>

enum InterpolationTypes
{
    nN = 0,
    bilinear = 1
};

class Texture 
{
public:
    Image *image;
    InterpolationTypes interpolationMode;

    Texture(const char* filename, InterpolationTypes interpolationMode)
    {
        image = new Image(filename);
        interpolationMode = interpolationMode;
    }

    Texture(InterpolationTypes interpolationMode)
    {
        this->interpolationMode = interpolationMode;
    }

    ~Texture()
    {
        delete image;
    }

    void BindImage(Image *image)
    {
        this->image = image;
    }



    glm::vec3 Fetch(float u, float v)
    {
        float i = u * image->width;
        float j = v * image->height;

        if(interpolationMode == InterpolationTypes::nN)
        {
            int iI = std::round(i);
            int iJ = std::round(j);

            return image->get(iI, iJ);
        }
        else if(interpolationMode == InterpolationTypes::bilinear)
        {

            int iI = std::floor(i);
            int iJ = std::floor(j);            

            float a = i - iI;
            float b = j - iJ;           

            return (1-a) * (1-b) * image->get(iI, iJ) +
                   (1-a) *   b   * image->get(iI, iJ + 1) +
                     a   * (1-b) * image->get(iI + 1, iJ) +
                     a   *   b   * image->get(iI + 1, iJ + 1);
        }
    }
};


#endif