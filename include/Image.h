#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stb_image.h>
#include <glm/glm.hpp>
#include <algorithm>

class Image {
public:
    int height, width, channels;
    unsigned char* pixels;
    Image(const char* filepath)
    {
        pixels = stbi_load(filepath, &width, &height, &channels, 0);
    }

    ~Image()
    {
        delete pixels;
    }

    glm::vec3 get(int i, int j)
    {

        int indexX = std::clamp(j, 0, height - 1) * width * channels;
        int indexY = std::clamp(i ,0, width - 1) * channels;

        float r = (int) pixels[indexX + indexY]     / 255.f;
        float g = (int) pixels[indexX + indexY + 1] / 255.f;
        float b = (int) pixels[indexX + indexY + 2] / 255.f;

        return glm::vec3(r,g,b);        
    }

};

#endif