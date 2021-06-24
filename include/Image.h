#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stb_image.h>
#include <tinyexr.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <string>
#include <iostream>

class Image {
public:
    int height, width, channels;
    float* pixels;
    Image(const char* filepath)
    {
        std::string path(filepath);
        std::string extensionName = path.substr(path.find_last_of(".")+1, path.size());

        if(extensionName == "exr")
        {
            const char* err = nullptr;
            int ret = LoadEXR(&pixels, &width, &height, filepath, &err);
            channels = 4;
            if(ret != 0)
            {
                if(err)
                {
                    fprintf(stderr, "ERR : %s\n", err);
                    FreeEXRErrorMessage(err);
                }
            }
        }
        else
        {
            unsigned char* tmpPixels;
            tmpPixels = stbi_load(filepath, &width, &height, &channels, 0);
            pixels = (float*)malloc(sizeof(float)*width*height*channels);

            for(int i=0; i<width*height*channels; i++)
                pixels[i] = (float)tmpPixels[i];

            free(tmpPixels);
        } 
    }

    ~Image()
    {
        delete pixels;
    }

    glm::vec3 get(int i, int j)
    {

        int indexX = std::clamp(j, 0, height - 1) * width * channels;
        int indexY = std::clamp(i ,0, width - 1) * channels;

        float r = pixels[indexX + indexY];
        float g = pixels[indexX + indexY + 1];
        float b = pixels[indexX + indexY + 2];

        return glm::vec3(r,g,b);        
    }

};

#endif