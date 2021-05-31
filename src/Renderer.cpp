#include <Renderer.h>


Renderer::Renderer(const std::string& filepath) : scene(std::string(ROOT_DIR) + filepath)
{

}

Renderer::~Renderer()
{

}

uint8_t* Renderer::FloatToUint8(float* pixels, int width, int height)
{
    // we will omit alpha channel
    uint8_t* res = new uint8_t[width * height * 3];

    int indexSrc = 0;
    int indexDst = 0;
    for(int j= 0; j<width; j++)
    {
        for(int i=0; i<height; i++)
        {
            float r = pixels[indexSrc++];
            float g = pixels[indexSrc++];
            float b = pixels[indexSrc++];
            indexSrc++;

            int ir = int(255.99 * r);
            int ig = int(255.99 * g);
            int ib = int(255.99 * b);

            res[indexDst++] = ir;
            res[indexDst++] = ig;
            res[indexDst++] = ib;
        }
    }

    return res;    
}

void Renderer::Render()
{
    uint8_t *result;

    scene.WritePixelCoord(2,2, glm::vec3(1.0, 0.0, 0.5));
    float* obtainedImage = scene.GetImage();

    result = FloatToUint8(obtainedImage, scene._imageWidth, scene._imageHeight);

    std::string outputPath = "outputs/" + scene._imageName;
    stbi_write_png(outputPath.c_str(), scene._imageWidth, scene._imageHeight, 3, result, scene._imageWidth *3);

    delete[] result;

}


