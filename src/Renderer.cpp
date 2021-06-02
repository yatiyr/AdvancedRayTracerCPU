#include <Renderer.h>


Renderer::Renderer(const std::string& filepath) : scene(std::string(ROOT_DIR) + filepath)
{

}

Renderer::~Renderer()
{

}

uint8_t* Renderer::GiveResult(uint8_t* pixels, int width, int height)
{
    // we will omit alpha channel
    uint8_t* res = new uint8_t[width * height * 3];

    int indexSrc = 0;
    int indexDst = 0;
    for(int j= 0; j<width; j++)
    {
        for(int i=0; i<height; i++)
        {
            int r = pixels[indexSrc++];
            int g = pixels[indexSrc++];
            int b = pixels[indexSrc++];
            indexSrc++;

            res[indexDst++] = r;
            res[indexDst++] = g;
            res[indexDst++] = b;
        }
    }

    return res;    
}

void Renderer::Render()
{
    uint8_t *result;

    scene.WritePixelCoord(2,2, glm::vec3(1.0, 0.0, 0.5));
    uint8_t* obtainedImage = scene.GetImage();

    result = GiveResult(obtainedImage, scene._imageWidth, scene._imageHeight);

    std::string outputPath = "outputs/" + scene._imageName;
    stbi_write_png(outputPath.c_str(), scene._imageWidth, scene._imageHeight, 3, result, scene._imageWidth *3);

    delete[] result;

}


