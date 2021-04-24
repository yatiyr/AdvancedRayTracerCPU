#ifndef __TIMER_H__
#define __TIMER_H__

#include <chrono>
#include <iostream>
#include <string>


// A SCOPE BASED TIMER
class Timer
{
private:
    std::string message;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

public:
    Timer()
    {
        startTime = std::chrono::high_resolution_clock::now();
    }

    Timer(std::string message)
    {
        this->message = message;
    }

    ~Timer()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();


        auto duration = end - start;

        double ms = duration * 0.001;

        std::cout << message << " " << ms << "ms (" << ms * 0.001 << "s)" << std::endl;

    }
};


#endif /* __TIMER_H__ */