#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <glm/mat4x4.hpp>
#include <Structures.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Texture.h>

class Object
{
public:
    glm::mat4 transformationMatrix;
    glm::mat4 transformationMatrixInversed;
    glm::mat4 transformationMatrixInverseTransposed;

    glm::vec3 translationVector;

    Texture *diffuseMap = nullptr;
    Texture *specularMap = nullptr;
    Texture *normalMap = nullptr;
    Texture *bumpMap = nullptr;
    Texture *emissionMap = nullptr;
    Texture *roughnessMap = nullptr;

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

    float ColorDistance(glm::vec3 c1, glm::vec3 c2)
    {
        long rmean = ((long)c1.x + (long)c2.x) / 2;
        long r = ((long)c1.x - (long)c2.x);
        long g = (long)c1.y - (long)c2.y;
        long b = (long)c1.z - (long)c2.z;
        return std::sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8))/5;
    }


};

#endif  /* __OBJECT_H__ */