#include <iostream>
#include <cmath>
#include <Renderer.h>

int main(int argc, char** argv)
{
    
    Renderer RnDr("assets/scenes/hw2/chinese_dragon.xml");

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