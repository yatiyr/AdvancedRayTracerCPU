#include <Renderer.h>

Renderer::Renderer(const std::string& filepath) : scene(std::string(ROOT_DIR) + filepath)
{
    contrast   = 0.5;
    brightness = 0.5;
}

Renderer::~Renderer()
{

}

void Renderer::SetConstrast(float val)
{
    contrast = val;
}

void Renderer::SetBrightness(float val)
{
    brightness = val;
}

uint8_t* Renderer::GiveResult(float* pixels, int width, int height)
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

            res[indexDst++] = r;
            res[indexDst++] = g;
            res[indexDst++] = b;
        }
    }

    return res;    
}

void Renderer::ToneMap(float* pixels, int width, int height)
{
    long double av_lum = 0;
    double max_lum = 0;

    for(int i=0; i<width*height; i++)
    {
        double r = pixels[i*3];
        double g = pixels[i*3 + 1];
        double b = pixels[i*3 + 2];


        double lum =  r*0.27 + g*0.67 + b*0.06;

        if(lum > max_lum)
            max_lum = lum;


        double testLum = std::log(0.00005 + lum);

        if(std::isnan(testLum))
            av_lum += 0;
        else
            av_lum += testLum;

    }

    max_lum = scene._activeCamera.burn_percentage * av_lum / 100;
    av_lum = std::pow(EULER, av_lum/(width*height));       

    for(int i=0; i<width*height; i++)
    {
        double r = pixels[i*3];
        double g = pixels[i*3 + 1];
        double b = pixels[i*3 + 2];

       
        double lum = r*0.27 + g*0.67 + b*0.06;

        double lm = (scene._activeCamera.keyValue * lum)/av_lum;

        double ld = lm *(1 + (lm/(max_lum*max_lum))/(scene._activeCamera.saturation + lm));

        //ld = std::clamp(ld, (float)0, (float)1);
                      
 
        // Luminance mapping
        pixels[i*3]     = ld * pixels[i*3] / lum;
        pixels[i*3 + 1] = ld * pixels[i*3 + 1] / lum;
        pixels[i*3 + 2] = ld * pixels[i*3 + 2] / lum;
        // Sigmoidal compression

        // Sigmoid 1
        //pixels[i*3]     = pixels[i*3]/(pixels[i*3] + 1);
        //pixels[i*3 + 1] = pixels[i*3 + 1]/(pixels[i*3 + 1] + 1);
        //pixels[i*3 + 2] = pixels[i*3 + 2]/(pixels[i*3 + 2] + 1);

        // Sigmoid 2
        //pixels[i*3] = (pixels[i*3]*(1 + pixels[i*3]/(max_lum*max_lum)))/(1 + pixels[i*3]);
        //pixels[i*3 + 1] = (pixels[i*3 + 1]*(1 + pixels[i*3 + 1]/(max_lum*max_lum)))/(1 + pixels[i*3 + 1]);
        //pixels[i*3 + 2] = (pixels[i*3 + 2]*(1 + pixels[i*3 + 2]/(max_lum*max_lum)))/(1 + pixels[i*3 + 2]);                

        //pixels[i*3] = ld_r;
        //pixels[i*3 + 1] = ld_g;
        //pixels[i*3 + 2] = ld_b;

        pixels[i*3]     = (std::pow((pixels[i*3]), 1/scene._activeCamera.gamma));
        pixels[i*3 + 1] = (std::pow((pixels[i*3 + 1]), 1/scene._activeCamera.gamma));
        pixels[i*3 + 2] = (std::pow((pixels[i*3 + 2]), 1/scene._activeCamera.gamma));           

        // gamma correction and scaling to [0, 255] range
        pixels[i*3]     *= 255 ;//* (std::pow((pixels[i*3]), scene._activeCamera.gamma));
        pixels[i*3 + 1] *= 255 ;//* (std::pow((pixels[i*3 + 1]), scene._activeCamera.gamma));
        pixels[i*3 + 2] *= 255 ;//* (std::pow((pixels[i*3 + 2]), scene._activeCamera.gamma));


        pixels[i*3]     = clamp(pixels[i*3], (float)0, (float)255);
        pixels[i*3 + 1] = clamp(pixels[i*3 + 1], (float)0, (float)255);
        pixels[i*3 + 2] = clamp(pixels[i*3 + 2], (float)0, (float)255);

    }

}

void Renderer::Clamp0_255(float* pixels, int width, int height)
{
    for(int i=0; i<width*height; i++)
    {
        pixels[i*3]     = clamp(pixels[i*3], (float)0, (float)255);
        pixels[i*3 + 1] = clamp(pixels[i*3 + 1], (float)0, (float)255);
        pixels[i*3 + 2] = clamp(pixels[i*3 + 2], (float)0, (float)255);
    }
}

void Renderer::WriteExr(float* rgb)
{
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    std::vector<float> images[3];
    images[0].resize(scene._imageWidth * scene._imageHeight);
    images[1].resize(scene._imageWidth * scene._imageHeight);
    images[2].resize(scene._imageWidth * scene._imageHeight);

    for(int i=0; i< scene._imageWidth * scene._imageHeight; i++)
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
    image.width  = scene._imageWidth;
    image.height = scene._imageHeight;

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

    std::string outputPath =  "outputs/" + scene._activeCamera.imageName;
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

void Renderer::RenderOneCamera()
{
    uint8_t *result;

    float* obtainedImage = scene.GetImage();

    if(scene._activeCamera.renderMode == RenderMode::CLASSIC)
    {
        std::string outputPath = "outputs/" + scene._activeCamera.imageName;
        Clamp0_255(obtainedImage, scene._activeCamera.imageResolution.x, scene._activeCamera.imageResolution.y);
        result = GiveResult(obtainedImage, scene._activeCamera.imageResolution.x, scene._activeCamera.imageResolution.y);
        stbi_write_png(outputPath.c_str(), scene._activeCamera.imageResolution.x, scene._activeCamera.imageResolution.y, 3, result, scene._activeCamera.imageResolution.x *3);        
    }
    else if(scene._activeCamera.renderMode == RenderMode::HDR)
    {
        std::string outputPath = "outputs/" + scene._activeCamera.imageName;
        WriteExr(obtainedImage);

        int dotIndex = outputPath.find('.');
        std::string pathWithoutExtension = outputPath.substr(0, dotIndex) + "_tonemapped.png"; 
        ToneMap(obtainedImage, scene._activeCamera.imageResolution.x, scene._activeCamera.imageResolution.y);
        result = GiveResult(obtainedImage, scene._activeCamera.imageResolution.x, scene._activeCamera.imageResolution.y);
        stbi_write_png(pathWithoutExtension.c_str(), scene._activeCamera.imageResolution.x, scene._activeCamera.imageResolution.y, 3, result, scene._activeCamera.imageResolution.x *3);        
    }

    delete[] result;

}

void Renderer::Render()
{
    for(size_t i=0; i<scene._cameras.size(); i++)
    {
        scene._activeCamera = scene._cameras[i];
        scene._imageHeight = scene._activeCamera.imageResolution.y;
        scene._imageWidth  = scene._activeCamera.imageResolution.x;

        float halfAperture = scene._activeCamera.apertureSize/2;

        scene.cameraVariableGenerator = new RandomGenerator(-halfAperture, halfAperture);

        RenderOneCamera();

        delete[] scene._image;
        scene._image = new float[scene._imageHeight*scene._imageWidth*3];
        scene.ClearImage();

    }
}


