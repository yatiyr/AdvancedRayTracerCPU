#include <iostream>
#include <Renderer.h>

int main(int argc, char** argv)
{

    std::string path = std::string("assets/scenes/cube_point_hdr.xml");

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