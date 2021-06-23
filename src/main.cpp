#include <iostream>
#include <Renderer.h>
#include <Texture.h>
#include <PerlinNoiseGenerator.h>


#include <tinyexr.h>

void WriteExr(float* rgb, int width, int height)
{
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    std::vector<float> images[3];
    images[0].resize(width * height);
    images[1].resize(width * height);
    images[2].resize(width * height);

    for(int i=0; i< width * height; i++)
    {
        images[0][i] = rgb[3*i + 0];
        images[1][i] = rgb[3*i + 1];
        images[2][i] = rgb[3*i + 2];
    } 

    float* image_ptr[3];
    image_ptr[0] = &(images[2].at(0));
    image_ptr[1] = &(images[1].at(0));
    image_ptr[2] = &(images[0].at(0));

    image.images = (unsigned char**) image_ptr;
    image.width  = width;
    image.height = height;

    header.num_channels = 3;
    header.channels     = (EXRChannelInfo *) malloc(sizeof(EXRChannelInfo) * header.num_channels);

    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';    

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for(int i=0; i<header.num_channels; i++)
    {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF;
    }

    std::string outputPath =  "outputs/test.png";
    int dotIndex = outputPath.find('.');
    std::string pathWithoutExtension = outputPath.substr(0, dotIndex) + ".exr";    
    const char* err;
    int ret = SaveEXRImageToFile(&image, &header, pathWithoutExtension.c_str(), &err);
    if(ret != TINYEXR_SUCCESS)
    {
        fprintf(stderr, "Save EXR err: %s\n", err);
        return;
    }

    printf("Saved exr file. [ %s ] \n", pathWithoutExtension.c_str());
    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
}

int main(int argc, char** argv)
{

    Renderer RnDr("assets/scenes/ellipsoids_texture.xml");

    /*
    std::string path = std::string(ROOT_DIR) + "assets/scenes/textures/grace-new.exr";
    path = "outputs/test.exr";
    float* out; // width * height * RGBA
    int width;
    int height;
    const char* err = nullptr; // or nullptr in C++11

    int ret = LoadEXR(&out, &width, &height, path.c_str(), &err);

    if (ret != 0) {
      if (err) {
        fprintf(stderr, "ERR : %s\n", err);
        FreeEXRErrorMessage(err); // release memory of error message.
      }
    } else {
      free(out); // release memory of image data
    } 

    float* image = (float *)malloc(sizeof(float) * 12);
    image[0] = 0;
    image[1] = 900;
    image[2] = 0.5;

    image[3] = -900;
    image[4] = 0;
    image[5] = 0;

    image[6] = 0;
    image[7] = 0.5;
    image[8] = 900;

    image[9] = 10000;
    image[10] = 0;
    image[11] = 0;

    WriteExr(image, 2, 2); */

    //stbi_set_flip_vertically_on_load(false);

/*
    PerlinNoiseGenerator generator;
    float xa = generator.GenerateNoise(glm::vec3(423.23, 180.12, -12.123));
    
    std::string filename = std::string(ROOT_DIR) + std::string("images/palette.png");
    Image *image = new Image(filename.c_str());

    Texture tex1(image, InterpolationType::BILINEAR, DecalMode::REPLACE_KD, TextureType::IMAGE);
    float ya = tex1.noiseGenerator.GenerateNoise(glm::vec3(423.23, 180.12, -12.123));
    Texture tex2(image, InterpolationType::NEAREST_NEIGHBOR, DecalMode::REPLACE_BACKGROUND, TextureType::IMAGE);

    glm::vec3 x = tex1.Fetch(-0.1,-0.1);
    glm::vec3 y = tex2.Fetch(0.1,0.1);
    glm::vec3 z = tex1.Fetch(1,0);
    glm::vec3 w = tex2.Fetch(1,0); */


    try
    {    
      RnDr.Render();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }


  return 0;
}