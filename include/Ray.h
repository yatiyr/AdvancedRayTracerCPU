#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>

class Ray
{
public:

    Ray(glm::vec3& origin, glm::vec3& direction)
    {
        this->origin = origin;
        this->direction = direction;

        this->invDirection = glm::vec3(1/direction.x, 1/direction.y, 1/direction.z);
        sign[0] = (invDirection.x < 0);
        sign[1] = (invDirection.y < 0);
        sign[2] = (invDirection.z < 0);

        isRefracting      = false;
        mediumCoeffBefore = 1;
        mediumCoeffNow    = 1;
    }

    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 invDirection;
    int sign[3];


    bool  isRefracting;

    float mediumCoeffBefore;
    float mediumCoeffNow;
};



#endif