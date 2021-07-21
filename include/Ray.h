#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>

class Ray
{
public:

    Ray()
    {

    }

    Ray(glm::vec3& origin, glm::vec3& direction)
    {
        this->origin = origin;
        this->direction = direction;
        this->lastHitPos = glm::vec3(-1.0);

        this->invDirection = glm::vec3(1/direction.x, 1/direction.y, 1/direction.z);
        sign[0] = (invDirection.x < 0);
        sign[1] = (invDirection.y < 0);
        sign[2] = (invDirection.z < 0);

        isRefracting      = false;
        mediumCoeffBefore = 1;
        mediumCoeffNow    = 1;
        rayEnergy         = 1;

        materialIdCurrentlyIn = -1;
    }

    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 invDirection;
    glm::vec3 lastHitPos;

    int sign[3];

    float mediumCoeffBefore;
    float mediumCoeffNow;
    float rayEnergy;
    float rayThroughput;
    
    int materialIdCurrentlyIn;

    bool  isRefracting;

    float time;
};



#endif