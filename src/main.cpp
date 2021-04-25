#include <iostream>
#include <cmath>
#include <Renderer.h>

#include <future>
#include <thread>

std::promise<int> p;
std::future<int> f = p.get_future();
std::shared_future<int> sf = f.share();


int var = 0;
std::mutex m;

int factorial(std::shared_future<int> f)
{
  int k = 1;

  int N = f.get();

  for(int i = N; i > 1; i--)
  {
    k *= i;
  }

  std::cout << "The factorial is = " << k << std::endl;
  return k;
}

void count()
{
  //std::cout << " x" << std::endl;

  while(true)
  {
    if(var >= 10)
      break;
    
    m.lock();
    int index = var;
    var++;
    m.unlock();
    std::cout << index << std::endl;
  }

}
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


/*
  int x;

  std::thread t1(count);
  std::thread t2(count);
  std::thread t3(count);

  t1.join();
  t2.join();
  t3.join();
*/
  return 0;
}