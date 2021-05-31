#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <Structures.h>
#include <Scene.h>
#include <stb_image.h>
#include <stb_image_write.h>

class Renderer
{
private:

    Scene scene;
    uint8_t* FloatToUint8(float* pixels, int width, int height);

public:
    Renderer(const std::string& filepath);
    ~Renderer();

    void Render();
};

#endif