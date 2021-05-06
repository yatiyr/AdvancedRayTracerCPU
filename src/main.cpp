#include <iostream>
#include <Renderer.h>

int main(int argc, char** argv)
{

    Renderer RnDr("assets/scenes/bunny.xml");

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