#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <glm/mat4x4.hpp>
#include <Structures.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Object
{
public:
    glm::mat4 transformationMatrix;
    glm::mat4 transformationMatrixInversed;
    glm::mat4 transformationMatrixInverseTransposed;

    glm::vec3 translationVector;

    virtual bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling) = 0;

    glm::mat4 MotionBlurTranslate(float time)
    {
        if(translationVector.x == 0 &&
           translationVector.y == 0 &&
           translationVector.z == 0)
           return glm::mat4(1.0f);

        glm::vec3 currentTranslation = time * translationVector;

        return glm::inverse(glm::translate(glm::mat4(1.0f),currentTranslation));

    }

};

#endif  /* __OBJECT_H__ */