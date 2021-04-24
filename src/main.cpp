#include <iostream>
//#include <Renderer.h>
#include <Timer.h>
#include <omp.h>

#include <cmath>
#include <Sphere.h>

int main(int argc, char** argv)
{
    
    //Renderer RnDr("assets/scenes/hw1/cornellbox.xml", RenderingMode::CPU_RENDERING_BIT);

    try
    {    
      //  RnDr.CPURender();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}