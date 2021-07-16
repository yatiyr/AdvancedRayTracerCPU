#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <Structures.h>
#include <Scene.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tinyexr.h>
#include <Utils.h>
#include <math.h>
#include <algorithm>

class Renderer
{
private:

    Scene scene;


    float contrast;
    float brightness;


    uint8_t* GiveResult(float* pixels, int width, int height);
    void ToneMap(float* pixels, int width, int height);
    void Clamp0_255(float* pixels, int width, int height);

public:
    Renderer(const std::string& filepath);
    ~Renderer();

    void RenderOneCamera();
    void Render();
    void WriteExr(float* rgb);

    void SetKeyValue(float val);
    void SetConstrast(float val);
    void SetGamma(float val);
    void SetBrightness(float val);
    void SetSaturation(float val);

};

#endif