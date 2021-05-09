#include <iostream>
#include <Renderer.h>

int main(int argc, char** argv)
{

    Renderer RnDr("assets/scenes/spheres_dof.xml");

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