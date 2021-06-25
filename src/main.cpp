#include <iostream>
#include <Renderer.h>

int main(int argc, char** argv)
{

    std::string path = std::string(argv[1]);

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