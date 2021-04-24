#include <iostream>
//#include <Renderer.h>
#include <Timer.h>
#include <omp.h>

#include <cmath>
#include <Renderer.h>

int main(int argc, char** argv)
{
    
    Renderer RnDr("assets/scenes/hw1/bunny.xml");

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