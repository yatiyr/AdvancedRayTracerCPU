#ifndef __DIRECTION_SAMPLER_H__
#define __DIRECTION_SAMPLER_H__

#include <RandomGenerator.h>
#include <Utils.h>

/**
 * This class generates random direction
 * for the hemisphere around a normal
 */
class DirectionSampler
{
private:
    RandomGenerator* randomGenerator;

public:
    DirectionSampler()
    {
        randomGenerator = new RandomGenerator(0.0f, 1.0f);
    }

    ~DirectionSampler()
    {
        delete randomGenerator;
    }

    // Directions similar to normal are more likely to be generated
    glm::vec3 importanceSample(glm::vec3 normal)
    {
        float randomNumber1 = randomGenerator->Generate();
        float randomNumber2 = randomGenerator->Generate();

        OrthonormalBasis basis = GiveOrthonormalBasis(normal);
        
        float localU = std::sqrt(randomNumber2)*std::cos(2*M_PI*randomNumber1);
        float localV = std::sqrt(1 - randomNumber2);
        float localW = std::sqrt(randomNumber2)*std::sin(2*M_PI*randomNumber1);

        glm::vec3 globalDirection = localU * basis.u + localV * normal + localW * basis.v;

        globalDirection = glm::normalize(globalDirection);
        return globalDirection;
    }

    // Every direction on the hemisphere has equal probability
    glm::vec3 uniformSample(glm::vec3 normal)
    {
        float randomNumber1 = randomGenerator->Generate();
        float randomNumber2 = randomGenerator->Generate();

        OrthonormalBasis basis = GiveOrthonormalBasis(normal);
        
        float localU = std::sqrt(1 - randomNumber2*randomNumber2)*std::cos(2*M_PI*randomNumber1);
        float localV = randomNumber2;
        float localW = std::sqrt(1 - randomNumber2*randomNumber2)*std::sin(2*M_PI*randomNumber1);

        glm::vec3 globalDirection = localU * basis.u + localV * normal + localW * basis.v;

        globalDirection = glm::normalize(globalDirection);
        return globalDirection;
    }
};

#endif