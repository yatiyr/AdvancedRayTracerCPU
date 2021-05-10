#include <Mesh.h>


Mesh::Mesh(const std::vector<Triangle>& triangleList, size_t materialId, bool softShadingFlag)
{
    this->bvhRoot = new BVH(triangleList, 0, 200, 0);
    this->materialId = materialId;
    this->softShadingFlag = softShadingFlag;
}

bool Mesh::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling)
{

    glm::mat4 motionBlurTranslationMatrix = MotionBlurTranslate(ray.time);


    glm::vec3 newOrigin = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(ray.origin, 1.0f));
    glm::vec3 newDirection = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(ray.direction, 0.0f));
    Ray newRay(newOrigin, newDirection);

    bool test = bvhRoot->Intersect(newRay, report, tmin, tmax, intersectionEpsilon, softShadingFlag, glm::transpose(motionBlurTranslationMatrix)*transformationMatrixInverseTransposed, backfaceCulling);

    report.materialId = materialId;
    if(test)
        report.intersection = ray.origin + ray.direction * report.d;

    return test;
}