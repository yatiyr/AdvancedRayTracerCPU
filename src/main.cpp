#include <iostream>
#include <Renderer.h>
#include <Texture.h>
#include <PerlinNoiseGenerator.h>

int main(int argc, char** argv)
{

    Renderer RnDr("assets/scenes/veachAjar.xml");


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