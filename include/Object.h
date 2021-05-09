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

virtual bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling) = 0;
};

#endif  /* __OBJECT_H__ */