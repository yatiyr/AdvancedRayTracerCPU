#include <iostream>
#include <Renderer.h>

int main(int argc, char** argv)
{

    std::string path = std::string("assets/scenes/cornellbox_jaroslav_path_diffuse.xml");

    Renderer RnDr(path);

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