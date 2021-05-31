#ifndef __RANDOM_GENERATOR_H__
#define __RANDOM_GENERATOR_H__

#include<random>

class RandomGenerator
{
private:
    float rangeFrom;
    float rangeTo;
    std::random_device randomDevice;
    std::mt19937       generator;
    std::uniform_real_distribution<float> distr;

public:
    RandomGenerator(float from, float to)
    {
        rangeFrom = from;
        rangeTo   = to;

        generator = std::mt19937(randomDevice());
        distr     = std::uniform_real_distribution<float>(rangeFrom, rangeTo);
    }

    float Generate()
    {
        return distr(generator);
    }
    
};

#endif